/******************************************************************************
  				成都度云万物科技有限公司
企业网址：www.doiot.net			企业淘宝：www.doiot.shop
QQ：3453844403	  					微信：do-iot
成都度云万物科技有限公司保留驱动代码版权，你可使用本代码进行移植、修改和开发，
但是原作者以及成都度云万物科技有限公司不对软件做担保和负责
******************************************************************************/
#include "M5310A.h"
#include <stdlib.h>

M5310A_t M5310A;

/*******************************************************************
* 函数名 : Uart1_Recv_Enable_Clear
* 描述   : 使能串口中断并清除端口上的残留的数据，避免端口上残留的杂乱数据影响到反馈
的状态信息的正常解析
* 输入   : 无
* 输出   : 无
* 返回   : 无
* 注意   : 
********************************************************************/
void M5310A_Recv_Enable_Clear(void)
{
//	memset(M5310A.RxBuffer, 0, M5310A_RX_SIZE);
	M5310A.RxIndex = 0;
	M5310A.RxDataStart = false;
	M5310A.ATInfo.ATState = AT_STATE_NULL;
	//使能串口中断并等待接收一个字节
	HAL_UART_Receive_IT(&M5310A_UART, &M5310A.uartBuffer, 1);
}

/*******************************************************************
* 函数名 : Send_At_Command
* 描述   : 将传入的命令发送到NB模块
* 输入   : cmd 需要发送的命令
					 cmd_id 对应命令的处理函数ID
					 at_type 消息类型:AT_NULL、AT_SET、AT_QUERY
* 输出   : 无
* 返回   : 0发送失败，1发送成功
* 注意   : 
********************************************************************/
bool Send_At_Command(char *cmd, AT_Command_t cmd_id, AT_Type_t at_type)
{
	bool res = false;
	uint16_t len = strlen(cmd);
	M5310A.ATInfo.ATState = AT_STATE_NULL;
	
	int num_send = 0;
	//当命令在命令的范围内，将命令发送出去，并将处理函数的id和命令类型
	//设置到ATInfo中
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
* 函数名 : M5310A_RxCallBack
* 描述   : 接收串口的回调函数，用来解析接收的数据并将数据交个AT指令
处理函数处理
* 输入   : 无
* 输出   : 无
* 返回   : 无
* 注意   : 
********************************************************************/
void M5310A_RxCallBack(void)
{	
	if(M5310A.ATInfo.ATCommand == AT_CMD_MIPLOPEN)
	{
		//printf("%c", M5310A.uartBuffer);
	}
	if(!M5310A.RxDataStart) {
		/*****读取到<CR><LF>*****/
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
			//当接收数据大于接收缓存强制进行数据处理解析，以保证当接收到大数据两的数据能够正确获取
			//并解析
			ATHandle(M5310A.RxBufferData, M5310A.RxDataIndex);
			memset(M5310A.RxBufferData, 0, M5310A_RX_DATA_SIZE);
			M5310A.RxDataIndex = 0;
		}
		
		/*****读取指令尾部<CR><LF>******/
		if(M5310A.RxDataIndex > 1 && \
			M5310A.RxBufferData[M5310A.RxDataIndex - 1] == '\n' && \
			M5310A.RxBufferData[M5310A.RxDataIndex - 2] == '\r')
		{
			if(M5310A.RxDataIndex > 2)
			{
				M5310A.RxDataIndex--;
				M5310A.RxDataStart = false;
				//调用相应处理函数
				ATHandle(M5310A.RxBufferData, M5310A.RxDataIndex);
				memset(M5310A.RxBufferData, 0, M5310A_RX_DATA_SIZE);
				M5310A.RxDataIndex = 0;
			} else
			{
				//处理防止丢包导致无法正确解析数据
				M5310A.RxDataIndex = 0;
			}
		}
	}
	//使能串口中断并等待接收一个字节
	HAL_UART_Receive_IT(&M5310A_UART, &M5310A.uartBuffer, 1);
}
/*******************************************************************
* 函数名 : Brd_M5310A_AT_MIPLCONF
* 描述   : 应用OneNet官方提供的mipl_generate_v1.2.c,mipl_generate()函数
-b Bootstrap 模式设置，1 为打开，0 为关闭;需要设置为 1

-e 加密模式设置，1 为打开，0 为关闭;目前不支持加密模式;

-d Debug 等级设置:
	0 关闭 debug 模式
	1 仅打印接收/发送包数据长度	
	2 打印发送数据包长度和接收数据内容及数据长度
	3 打印接收/发送数据内容及数据长度

-i host 设置，格式为”ServerURI:Port”;
	ServerURI 远端服务器地址，重庆平台使用 183.230.40.39
	Port 远端服务器端口号，重庆平台使用 5683

-n 鉴权参数，格式为”IMEI;IMSI”，对应平台侧设备注册时的鉴
权参数 IMEI 和 IMSI;

-p 本端端口号，范围 0~65535;缺省值 0，当选择缺省时，模组
会自动从 32768~65535 中选择一个可用的端口号;

-t 设备存活时间，标示终端和 OneNET 平台之间连接的存活周
期，设置范围为 10s~86400s;

-u 设置 PUT 和 POST 指令分片长度，范围 0~6，指示分片长度
为 2^(4+u)，缺省值为 5;

-g 设置 GET 指令分片长度，范围 0~6，指示分片长度为 2^(4+g),
缺省值为 5;

-x 设置触发分片操作的最大长度，范围 0~2，指示阈值为
2^(4+x)，缺省值为 2。
* 输入   : 无
* 输出   : 无
* 返回   : 无
* 注意   : 
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
* 函数名 : Wait_Rec_AT
* 描述   : 等待发送AT指令后返回的状态，如果超时和返回的错误状态则返回
失败
* 输入   : timeout 超时时间单位为ms
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
********************************************************************/
bool Wait_Rec_AT(uint32_t timeout)
{
	bool res = false;
	//在等待状态前获取起始时间
	uint32_t start_time = HAL_GetTick();
	
	//等待返回的状态信息直到超时
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
* 函数名 : Brd_M5310A_Snd_AT
* 描述   : 发送AT，返回OK，测试串口和M5310A模块之间通信是否正常
* 输入   : 无
* 输出   : 无
* 返回   : 0 : 失败   非零 ：成功
* 注意   : 发送AT，返回OK
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
* 函数名 : MESetAtState
* 描述   : 设置AT指令执行后的状态
* 输入   : stat需要设置的状态，有多种状态
* 输出   : 无
* 返回   : 无
* 注意   : 
********************************************************************/
void MESetAtState(AT_State_t state)
{
	M5310A.ATInfo.ATState = state;
	M5310A.ATInfo.ATCommand = AT_CMD_NULL;
}
/*******************************************************************
* 函数名 : Brd_M5310A_AT_CSQ
* 描述   : 查询信号强度
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_CEREG
* 描述   : 查询网络附着状态
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_GSN
* 描述   : 查询IMEI号
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_CIMI
* 描述   : 查询IMSI号
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_MIPLADDOBJ
* 描述   : 添加LWM2M object组
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_MIPLNOTIFY
* 描述   : 
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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

	//当发送的数据大于最大的发送缓冲区长度直接返回失败
	
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
* 函数名 : Brd_M5310A_AT_MIPLOPEN
* 描述   : 
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_MIPLNOTIFY
* 描述   : 
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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

	//当发送的数据大于最大的发送缓冲区长度直接返回失败
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
* 函数名 : Brd_M5310A_AT_MIPLCLOSE
* 描述   : 关闭连接
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_MMIPLDELOBJ
* 描述   : 
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : Brd_M5310A_AT_MMIPLDEL
* 描述   : 
* 输入   : 无
* 输出   : 无
* 返回   : 0失败，1成功
* 注意   : 
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
* 函数名 : M5310A_Api_Init
* 描述   : 开机并初始化注册电信平台
* 输入   : 无
* 输出   : 无
* 返回   : 0 : 失败   非零 ：成功
* 注意   : 
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
				//确认通讯正常
				res = Brd_M5310A_Snd_AT();
				break;
			case 1:
			{
				//确定信号质量良好
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
				//查询网络附着状态
				res = Brd_M5310A_AT_CEREG();
				break;
			case 3:
				//获取IMEI号
				res = Brd_M5310A_AT_CGSN();
				printf("IMEI:%llu\n", M5310A.imei);
				break;
			case 4:
				//获取IMSI号
				res = Brd_M5310A_AT_CIMI();
				printf("IMSI:%llu\n", M5310A.imsi);
				break;
			case 5:
				//模块侧设备创建
				res = Brd_M5310A_AT_MIPLCONF();
				break;
			case 6:
				//订阅Object组
				res = Brd_M5310A_AT_MIPLADDOBJ(0,3200,0);
				break;
			case 7:
				//订阅Resource
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5505,6,"A309C82FE6",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5500,5,"1",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5750,1,"juidl",1);								
				break;
			case 8:				
				//模块侧发起登录请求
				res = Brd_M5310A_AT_MIPLOPEN(0,60);
				printf("res：%d\n",res);
			//	HAL_Delay(40000);
				break;
			case 9:
				//订阅Resource
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5505,6,"A309C82FE6",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5500,5,"1",1);
				res = Brd_M5310A_AT_MIPLNOTIFY(0,3200,0,5750,1,"scnu2020",1);
				break;
			default:
				break;
		}
		
		//成功就移动下一步骤，失败则重新继续当超过最大
		//失败次数，或者完成初始化步骤将退出循环并返回
		//初始化的状态
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
* 函数名 : M5310A_exit
* 描述   : 退出透传模式
* 输入   : 无
* 输出   : 无
* 返回   : 0 : 失败   非零 ：成功
* 注意   : 
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
				//模块侧发起注销
				res = Brd_M5310A_AT_MIPLCLOSE(0);
				break;
			case 1:
				//删除模组订阅的本地 3200 Object
				res = Brd_M5310A_AT_MMIPLDELOBJ(0,3200,0);
				break;
			case 2:
				//销毁模组现存的通信实例
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
