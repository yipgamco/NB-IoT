/******************************************************************************
  				�ɶ���������Ƽ����޹�˾
��ҵ��ַ��www.doiot.net			��ҵ�Ա���www.doiot.shop
QQ��3453844403	  					΢�ţ�do-iot
�ɶ���������Ƽ����޹�˾�������������Ȩ�����ʹ�ñ����������ֲ���޸ĺͿ�����
����ԭ�����Լ��ɶ���������Ƽ����޹�˾��������������͸���
******************************************************************************/
#include "M5310A.h"
#include <stdlib.h>

M5310A_t M5310A;

/*******************************************************************
* ������ : Uart1_Recv_Enable_Clear
* ����   : ʹ�ܴ����жϲ�����˿��ϵĲ��������ݣ�����˿��ϲ�������������Ӱ�쵽����
��״̬��Ϣ����������
* ����   : ��
* ���   : ��
* ����   : ��
* ע��   : 
********************************************************************/
void M5310A_Recv_Enable_Clear(void)
{
//	memset(M5310A.RxBuffer, 0, M5310A_RX_SIZE);
	M5310A.RxIndex = 0;
	M5310A.RxDataStart = false;
	M5310A.ATInfo.ATState = AT_STATE_NULL;
	//ʹ�ܴ����жϲ��ȴ�����һ���ֽ�
	HAL_UART_Receive_IT(&M5310A_UART, &M5310A.uartBuffer, 1);
}

/*******************************************************************
* ������ : Send_At_Command
* ����   : �����������͵�NBģ��
* ����   : cmd ��Ҫ���͵�����
					 cmd_id ��Ӧ����Ĵ�����ID
					 at_type ��Ϣ����:AT_NULL��AT_SET��AT_QUERY
* ���   : ��
* ����   : 0����ʧ�ܣ�1���ͳɹ�
* ע��   : 
********************************************************************/
bool Send_At_Command(char *cmd, AT_Command_t cmd_id, AT_Type_t at_type)
{
	bool res = false;
	uint16_t len = strlen(cmd);
	M5310A.ATInfo.ATState = AT_STATE_NULL;
	
	int num_send = 0;
	//������������ķ�Χ�ڣ�������ͳ�ȥ��������������id����������
	//���õ�ATInfo��
	while(len > M5310A_TX_SIZE)
	{
		memset(M5310A.TxBuffer, 0, M5310A_TX_SIZE);
		memcpy(M5310A.TxBuffer, cmd+num_send*M5310A_TX_SIZE, M5310A_TX_SIZE);
		HAL_UART_Transmit_IT(&M5310A_UART, M5310A.TxBuffer, M5310A_TX_SIZE);
		HAL_Delay(100);
		len -= M5310A_TX_SIZE;
		num_send++;
	}
	
//	if(len < M5310A_TX_SIZE)
//	{
	memset(M5310A.TxBuffer, 0, M5310A_TX_SIZE);
	memcpy(M5310A.TxBuffer, cmd+num_send*M5310A_TX_SIZE, len);
	if(HAL_UART_Transmit_IT(&M5310A_UART, M5310A.TxBuffer, len) == HAL_OK)
	{
		M5310A.ATInfo.ATCommand = cmd_id;
		M5310A.ATInfo.ATType = at_type;
		M5310A.TxDataLastTime = HAL_GetTick();			
		res = true;
	}
	else
		res = false;
//	}
	
	return res;
}

/*******************************************************************
* ������ : M5310A_RxCallBack
* ����   : ���մ��ڵĻص������������������յ����ݲ������ݽ���ATָ��
����������
* ����   : ��
* ���   : ��
* ����   : ��
* ע��   : 
********************************************************************/
void M5310A_RxCallBack(void)
{	
	if(M5310A.ATInfo.ATCommand == AT_CMD_MIPLOPEN)
	{
		//printf("%c", M5310A.uartBuffer);
	}
	if(!M5310A.RxDataStart) {
		/*****��ȡ��<CR><LF>*****/
		M5310A.RxBufferData[M5310A.RxIndex] = M5310A.uartBuffer;
		
		if(M5310A.RxIndex < (M5310A_RX_DATA_SIZE - 1))
			M5310A.RxIndex++;
		else
			M5310A.RxIndex = 0;
		
		if(M5310A.RxIndex > 1 \
			&& (M5310A.RxBufferData[M5310A.RxIndex - 1] == '\n') \
			&& (M5310A.RxBufferData[M5310A.RxIndex - 2] == '\r'))
		{
			M5310A.RxDataStart = true;
			M5310A.RxIndex = 0;
		}
	}
	else
	{			
		M5310A.RxBufferData[M5310A.RxDataIndex] = M5310A.uartBuffer;
		
		if(M5310A.RxDataIndex == 0 && M5310A.uartBuffer == '\n' && M5310A.uartBuffer == '\r')
		{
			M5310A.RxDataIndex = 0;
		}
		else if(M5310A.RxDataIndex < (M5310A_RX_DATA_SIZE - 1))
		{
			M5310A.RxDataIndex++;
		}
		else if (M5310A.RxDataIndex >= (M5310A_RX_DATA_SIZE - 1))
		{
			M5310A.RxDataIndex--;
			//���������ݴ��ڽ��ջ���ǿ�ƽ������ݴ���������Ա�֤�����յ����������������ܹ���ȷ��ȡ
			//������
			ATHandle(M5310A.RxBufferData, M5310A.RxDataIndex);
			memset(M5310A.RxBufferData, 0, M5310A_RX_DATA_SIZE);
			M5310A.RxDataIndex = 0;
		}
		
		/*****��ȡָ��β��<CR><LF>******/
		if(M5310A.RxDataIndex > 1 && \
			M5310A.RxBufferData[M5310A.RxDataIndex - 1] == '\n' && \
			M5310A.RxBufferData[M5310A.RxDataIndex - 2] == '\r')
		{
			if(M5310A.RxDataIndex > 2)
			{
				M5310A.RxDataIndex--;
				M5310A.RxDataStart = false;
				//������Ӧ������
				ATHandle(M5310A.RxBufferData, M5310A.RxDataIndex);
				memset(M5310A.RxBufferData, 0, M5310A_RX_DATA_SIZE);
				M5310A.RxDataIndex = 0;
			} else
			{
				//�����ֹ���������޷���ȷ��������
				M5310A.RxDataIndex = 0;
			}
		}
	}
	//ʹ�ܴ����жϲ��ȴ�����һ���ֽ�
	HAL_UART_Receive_IT(&M5310A_UART, &M5310A.uartBuffer, 1);
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MIPLCONF
* ����   : Ӧ��OneNet�ٷ��ṩ��mipl_generate_v1.2.c,mipl_generate()����
-b Bootstrap ģʽ���ã�1 Ϊ�򿪣�0 Ϊ�ر�;��Ҫ����Ϊ 1

-e ����ģʽ���ã�1 Ϊ�򿪣�0 Ϊ�ر�;Ŀǰ��֧�ּ���ģʽ;

-d Debug �ȼ�����:
	0 �ر� debug ģʽ
	1 ����ӡ����/���Ͱ����ݳ���	
	2 ��ӡ�������ݰ����Ⱥͽ����������ݼ����ݳ���
	3 ��ӡ����/�����������ݼ����ݳ���

-i host ���ã���ʽΪ��ServerURI:Port��;
	ServerURI Զ�˷�������ַ������ƽ̨ʹ�� 183.230.40.39
	Port Զ�˷������˿ںţ�����ƽ̨ʹ�� 5683

-n ��Ȩ��������ʽΪ��IMEI;IMSI������Ӧƽ̨���豸ע��ʱ�ļ�
Ȩ���� IMEI �� IMSI;

-p ���˶˿ںţ���Χ 0~65535;ȱʡֵ 0����ѡ��ȱʡʱ��ģ��
���Զ��� 32768~65535 ��ѡ��һ�����õĶ˿ں�;

-t �豸���ʱ�䣬��ʾ�ն˺� OneNET ƽ̨֮�����ӵĴ����
�ڣ����÷�ΧΪ 10s~86400s;

-u ���� PUT �� POST ָ���Ƭ���ȣ���Χ 0~6��ָʾ��Ƭ����
Ϊ 2^(4+u)��ȱʡֵΪ 5;

-g ���� GET ָ���Ƭ���ȣ���Χ 0~6��ָʾ��Ƭ����Ϊ 2^(4+g),
ȱʡֵΪ 5;

-x ���ô�����Ƭ��������󳤶ȣ���Χ 0~2��ָʾ��ֵΪ
2^(4+x)��ȱʡֵΪ 2��
* ����   : ��
* ���   : ��
* ����   : ��
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MIPLCONF(void)
{
	bool res = false;
	int length = 0;
	char temp = 0;
	char *miplconf_buf;
	char buf[80];
	uint32_t offset = 0;
	memset(buf, 0, sizeof(buf));
	
	MIPL_DEBUG_LEVEL_E debug;
	uint16_t port;
	uint32_t keep_alive;
	size_t uri_len;
	char *uri;
	size_t ep_len;
	char *ep;
	
	length = snprintf(&temp, 1 , "coap://%s:%s", SERVER_IP, SERVER_PORT);
	if ((uri = (char *)malloc(sizeof(char)*length)) == NULL)
		return res;
	snprintf(uri, length + 1, "coap://%s:%s", SERVER_IP, SERVER_PORT);
	uri_len = strlen(uri);
	port = 0;
	keep_alive = 3000;

	length = snprintf(&temp, 1, "%llu;%llu", M5310A.imei, M5310A.imsi);
	if ((ep = (char *)malloc(sizeof(char)* length)) == NULL)
		return res;
	snprintf(ep, length + 1, "%llu;%llu", M5310A.imei, M5310A.imsi);
	ep_len = strlen(ep);
	debug = MIPL_DEBUG_LEVEL_NONE;

	char mode = ((1 & 0x1) << 4) | (0 & 0x1);
	memcpy(buf + offset, &mode, 1);
	offset++;
	memcpy(buf + offset, &debug, 1);
	offset++;
	memcpy(buf + offset, &port, 2);
	offset += 2;
	memcpy(buf+offset, &keep_alive, 4);
	offset += 4;

	memcpy(buf + offset, &uri_len, 2);
	offset += 2;
	memcpy(buf + offset, uri, uri_len);
	offset += uri_len;
	memcpy(buf + offset, &ep_len, 2);
	offset += 2;
	memcpy(buf + offset, ep, ep_len);
	offset += ep_len;

	*(buf + offset) = 5;
	offset++;
	*(buf + offset) = 5;
	offset++;
	*(buf + offset) = 1;
	offset++;
	
	free(uri);
	uri = NULL;
	free(ep);
	ep = NULL;
	
	char buf_temp[160];
	uint16_t i = 0;
	while (i < offset)
	{
		uint16_t a = i;
		sprintf(&buf_temp[a*2] ,"%02X", buf[i++]);
	}
	
	length = snprintf(&temp, 1, "AT+MIPLCONF=%d,%s,%d,%d\r\n", offset, buf_temp, 1, 1)+1;
	
	if((miplconf_buf = (char *)malloc(sizeof(char)* length)) == NULL)
		return res;
	
	snprintf(miplconf_buf, length+1, "AT+MIPLCONF=%d,%s,%d,%d\r\n", offset, buf_temp, 1, 1);
	/*
	if (Send_At_Command(miplconf_buf, AT_CMD_MIPLCONF, AT_SET))
		res = Wait_Rec_AT(10000);
	else
		res = false;
	*/
	if (Send_At_Command(miplconf_buf, AT_CMD_MIPLCONF, AT_SET))
		res = true;
	
	free(miplconf_buf);
	miplconf_buf = NULL;
	
	return res;
}

/*******************************************************************
* ������ : Wait_Rec_AT
* ����   : �ȴ�����ATָ��󷵻ص�״̬�������ʱ�ͷ��صĴ���״̬�򷵻�
ʧ��
* ����   : timeout ��ʱʱ�䵥λΪms
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Wait_Rec_AT(uint32_t timeout)
{
	bool res = false;
	//�ڵȴ�״̬ǰ��ȡ��ʼʱ��
	uint32_t start_time = HAL_GetTick();
	
	//�ȴ����ص�״̬��Ϣֱ����ʱ
	while(1)
	{
		if(((HAL_GetTick() - start_time) > timeout) || (M5310A.ATInfo.ATState == AT_STATE_ERR))
		{
			res = false;
			break;
		}		
		else if(M5310A.ATInfo.ATState == AT_STATE_OK)
		{
			res = true;
			break;
		}
	}
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_Snd_AT
* ����   : ����AT������OK�����Դ��ں�M5310Aģ��֮��ͨ���Ƿ�����
* ����   : ��
* ���   : ��
* ����   : 0 : ʧ��   ���� ���ɹ�
* ע��   : ����AT������OK
********************************************************************/
bool Brd_M5310A_Snd_AT(void)
{
	bool res = false;
	
	if (Send_At_Command("AT\r\n", AT_CMD_AT, AT_QUERY))
		res = Wait_Rec_AT(5000);
	else
		res = false;
	
	return res;
}

/*******************************************************************
* ������ : MESetAtState
* ����   : ����ATָ��ִ�к��״̬
* ����   : stat��Ҫ���õ�״̬���ж���״̬
* ���   : ��
* ����   : ��
* ע��   : 
********************************************************************/
void MESetAtState(AT_State_t state)
{
	M5310A.ATInfo.ATState = state;
	M5310A.ATInfo.ATCommand = AT_CMD_NULL;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_CSQ
* ����   : ��ѯ�ź�ǿ��
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_CSQ(void)
{
	bool res = false;
	
	if (Send_At_Command("AT+CSQ\r\n", AT_CMD_CSQ, AT_QUERY))
		res = Wait_Rec_AT(10000);
	else
		res = false;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_CEREG
* ����   : ��ѯ���總��״̬
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_CEREG(void)
{
	bool res = false;
	
	if (Send_At_Command("AT+CEREG?\r\n", AT_CMD_CEREG, AT_QUERY))
		res = Wait_Rec_AT(5000);
	else
		res = false;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_GSN
* ����   : ��ѯIMEI��
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_CGSN(void)
{
	bool res = false;

	if(Send_At_Command("AT+CGSN=1\r\n", AT_CMD_CGSN, AT_QUERY))
		res = Wait_Rec_AT(5000);
	else
		res = false;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_CIMI
* ����   : ��ѯIMSI��
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_CIMI(void)
{
	bool res = false;

	if(Send_At_Command("AT+CIMI\r\n", AT_CMD_CIMI, AT_QUERY))
		res = Wait_Rec_AT(5000);
	else
		res = false;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MIPLADDOBJ
* ����   : ���LWM2M object��
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MIPLADDOBJ(int ref, int16_t objectid, int instanceid)
{
	bool res = false;
	int length = 0;
	char temp;
	char *COM_BUF = NULL;

	length = snprintf(&temp, 1, "AT+MIPLADDOBJ=%d,%d,%d\r\n", ref, objectid, instanceid);
	if((COM_BUF = (char *)malloc(sizeof(char)*length)) == NULL)
		return res;
	
	snprintf(COM_BUF, length, "AT+MIPLADDOBJ=%d,%d,%d\r\n", ref, objectid, instanceid);

	if(Send_At_Command(COM_BUF, AT_CMD_MIPLADDOBJ, AT_SET))
		res = Wait_Rec_AT(5000);
	else
		res = false;
	
	free(COM_BUF);
	COM_BUF = NULL;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MIPLNOTIFY
* ����   : 
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MIPLNOTIFY(int8_t ref, 
															uint16_t objectid,
															int8_t instanceid,
															uint16_t resourceid,
															int8_t valuetype,
															char *value,
															bool flag
															)
{
	bool res = false;
	int length = 0;
	char temp;
	char *COM_BUF = NULL;

	//�����͵����ݴ������ķ��ͻ���������ֱ�ӷ���ʧ��
	
	length = snprintf(&temp, 1, "AT+MIPLNOTIFY=%d,%d,%d,%d,%d,\"%s\",%d\r\n", \
					ref, objectid, instanceid, resourceid, valuetype, value, flag) + 1;
	
	
	if((COM_BUF = (char *)malloc(sizeof(char)*length)) == NULL)		
		return res;
	
	snprintf(COM_BUF, length, "AT+MIPLNOTIFY=%d,%d,%d,%d,%d,\"%s\",%d\r\n", \
					ref, objectid, instanceid, resourceid, valuetype, value, flag);

	if(Send_At_Command(COM_BUF, AT_CMD_MIPLNOTIFY, AT_SET))
		res = Wait_Rec_AT(5000);
	else
		res = false;
	
	free(COM_BUF);
	COM_BUF = NULL;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MIPLOPEN
* ����   : 
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MIPLOPEN(int ref, int16_t timeout)
{
	bool res = false;
	int length = 0;
	char temp;
	char *COM_BUF = NULL;

	length = snprintf(&temp, 1, "AT+MIPLOPEN=%d,%d\r\n", ref, timeout) + 1;
	if((COM_BUF = (char *)malloc(sizeof(char)*length)) == NULL)
		return res;
	
	snprintf(COM_BUF, length, "AT+MIPLOPEN=%d,%d\r\n", ref, timeout);

	if(Send_At_Command(COM_BUF, AT_CMD_MIPLOPEN, AT_SET))
		res = Wait_Rec_AT(60000);
	else
		res = false;
	
	free(COM_BUF);
	COM_BUF = NULL;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MIPLNOTIFY
* ����   : 
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MIPLREAD(int8_t ref, 
														int16_t msgid,
														uint16_t objectid,
														int8_t instanceid,
														uint16_t resourceid,
														int8_t valuetype,
														char *value,
														bool flag
														)
{
	bool res = false;
	char COM_BUF[M5310A_TX_SIZE];

	//�����͵����ݴ������ķ��ͻ���������ֱ�ӷ���ʧ��
	if(strlen(value) > M5310A_SEND_MAX)
	{
		return res;
	}
	
	snprintf(COM_BUF, sizeof(COM_BUF), "AT+MIPLREAD=%d,%d,%d,%d,%d,%d,\"%s\",%d\r\n", \
					ref, msgid, objectid, instanceid, resourceid, valuetype, value, flag);

	if(Send_At_Command(COM_BUF, AT_CMD_MIPLREAD, AT_SET))
		res = Wait_Rec_AT(5000);
	else
		res = false;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MIPLCLOSE
* ����   : �ر�����
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MIPLCLOSE(int ref)
{
	bool res = false;
	int length = 0;
	char temp;
	char *COM_BUF = NULL;

	length = snprintf(&temp, 1, "AT+MIPLCLOSE=%d\r\n", ref) + 1;
	if((COM_BUF = (char *)malloc(sizeof(char)*length)) == NULL)
		return res;
	
	snprintf(COM_BUF, length, "AT+MIPLCLOSE=%d\r\n", ref);
	if(Send_At_Command(COM_BUF, AT_CMD_MIPLCLOSE, AT_SET))
		res = Wait_Rec_AT(60000);
	else
		res = false;
	
	free(COM_BUF);
	COM_BUF = NULL;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MMIPLDELOBJ
* ����   : 
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MMIPLDELOBJ(int8_t ref, int16_t objectid, int8_t instanceid)
{
	bool res = false;
	int length = 0;
	char temp;
	char *COM_BUF = NULL;

	length = snprintf(&temp, 1, "AT+MIPLDELOBJ=%d,%d,%d\r\n", ref, objectid, instanceid) + 1;
	if((COM_BUF = (char *)malloc(sizeof(char)*length)) == NULL)
		return res;
	
	snprintf(COM_BUF, length, "AT+MIPLDELOBJ=%d,%d,%d\r\n", ref, objectid, instanceid);
	
	if(Send_At_Command(COM_BUF, AT_CMD_MIPLDELOBJ, AT_SET))
		res = Wait_Rec_AT(120000);
	else
		res = false;
	
	free(COM_BUF);
	COM_BUF = NULL;
	
	return res;
}
/*******************************************************************
* ������ : Brd_M5310A_AT_MMIPLDEL
* ����   : 
* ����   : ��
* ���   : ��
* ����   : 0ʧ�ܣ�1�ɹ�
* ע��   : 
********************************************************************/
bool Brd_M5310A_AT_MMIPLDEL(int8_t ref)
{
	bool res = false;
	int length = 0;
	char temp;
	char *COM_BUF = NULL;

	length = snprintf(&temp, 1, "AT+MIPLDEL=%d\r\n", ref) + 1;
	if((COM_BUF = (char *)malloc(sizeof(char)*length)) == NULL)
		return res;
	
	snprintf(COM_BUF, length, "AT+MIPLDEL=%d\r\n", ref);
	
	if(Send_At_Command(COM_BUF, AT_CMD_MIPLDEL, AT_SET))
		res = Wait_Rec_AT(120000);
	else
		res = false;
	
	free(COM_BUF);
	COM_BUF = NULL;
	
	return res;
}

void M5310A_Hardware_Init(void){
	HAL_GPIO_WritePin(RESET_IO_GPIO_Port, RESET_IO_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(RESET_IO_GPIO_Port, RESET_IO_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);

}



/*******************************************************************
* ������ : M5310A_Api_Init
* ����   : ��������ʼ��ע�����ƽ̨
* ����   : ��
* ���   : ��
* ����   : 0 : ʧ��   ���� ���ɹ�
* ע��   : 
********************************************************************/
bool M5310A_Api_Init(void)
{
	bool res = false;
	uint8_t Init_Step = 0;
	uint8_t Init_Count = M5310A_INIT_MAX_TIMES;
	
	while(1)
	{
		switch(Init_Step)
		{
			case 0:
				M5310A_Recv_Enable_Clear();
				//ȷ��ͨѶ����
				res = Brd_M5310A_Snd_AT();
				break;
			case 1:
			{
				//ȷ���ź���������
				uint8_t Wait_Rssi_Count = 0;
				do
				{
					res = Brd_M5310A_AT_CSQ();
					if (res)
					{
						if(M5310A.Rssi > 33 || M5310A.Rssi <= 0)
						{
							HAL_Delay(5000);
							Wait_Rssi_Count++;
						}
						else if(M5310A.Rssi <= 33 && M5310A.Rssi > 0)
						{
							printf("rssi:%d\n", M5310A.Rssi);
							break;
						}
					}
				}while(res && (Wait_Rssi_Count < 4));
				
				if(res && M5310A.Rssi > 33)
				{
					res = false;
				}
				break;
			}
			case 2:
				//��ѯ���總��״̬
				res = Brd_M5310A_AT_CEREG();
				break;
			case 3:
				//��ȡIMEI��
				res = Brd_M5310A_AT_CGSN();
				printf("IMEI:%llu\n", M5310A.imei);
				break;
			case 4:
				//��ȡIMSI��
				res = Brd_M5310A_AT_CIMI();
				printf("IMSI:%llu\n", M5310A.imsi);
				break;
			case 5:
				//ģ����豸����
				res = Brd_M5310A_AT_MIPLCONF();
				break;
			case 6:
				//����Object��
				res = Brd_M5310A_AT_MIPLADDOBJ(0,3200,0);
				break;
			case 7:
				//����Resource
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5505,6,"A309C82FE6",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5500,5,"1",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5750,1,"juidl",1);								
				break;
			case 8:				
				//ģ��෢���¼����
				res = Brd_M5310A_AT_MIPLOPEN(0,60);
				printf("res��%d\n",res);
			//	HAL_Delay(40000);
				break;
			case 9:
				//����Resource
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5505,6,"A309C82FE6",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5500,5,"1",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5750,1,"scnu2020",1);
				break;
			default:
				break;
		}
		
		//�ɹ����ƶ���һ���裬ʧ�������¼������������
		//ʧ�ܴ�����������ɳ�ʼ�����轫�˳�ѭ��������
		//��ʼ����״̬
		if(!res)
		{
			Init_Count--;	
			if(Init_Count == 0)
			{
				printf("Fail at step %d\n", Init_Step);
				break;
			}
		}
		else
		{
			Init_Step++;
			if(Init_Step > 9)
			{
				break;
			}
		}
	}
	
	return res;
}
/*******************************************************************
* ������ : M5310A_exit
* ����   : �˳�͸��ģʽ
* ����   : ��
* ���   : ��
* ����   : 0 : ʧ��   ���� ���ɹ�
* ע��   : 
********************************************************************/
bool M5310A_exit(void)
{
	bool res = false;
	uint8_t Exit_Step = 0;
	uint8_t Exit_Count = M5310A_EXIT_MAX_TIMES;
	
	while(1)
	{
		switch(Exit_Step)
		{
			case 0:
				//ģ��෢��ע��
				res = Brd_M5310A_AT_MIPLCLOSE(0);
				break;
			case 1:
				//ɾ��ģ�鶩�ĵı��� 3200 Object
				res = Brd_M5310A_AT_MMIPLDELOBJ(0,3200,0);
				break;
			case 2:
				//����ģ���ִ��ͨ��ʵ��
				res = Brd_M5310A_AT_MMIPLDEL(0);
				break;
		}
		
		if(!res)
		{
			Exit_Count--;	
			if(Exit_Count == 0)
				break;
		}
		else
		{
			Exit_Step++;
			if(Exit_Step > 2)
			{
				break;
			}
		}
	}
	
	return res;
}
