/******************************************************************************
  				�ɶ���������Ƽ����޹�˾
��ҵ��ַ��www.doiot.net			��ҵ�Ա���www.doiot.shop
QQ��3453844403	  					΢�ţ�do-iot
�ɶ���������Ƽ����޹�˾�������������Ȩ�����ʹ�ñ����������ֲ���޸ĺͿ�����
����ԭ�����Լ��ɶ���������Ƽ����޹�˾��������������͸���
******************************************************************************/
#ifndef __M5310A_H__
#define __M5310A_H__
#include "main.h"

extern UART_HandleTypeDef huart2;

#define M5310A_UART	huart2
#define M5310A_INIT_MAX_TIMES 10
#define M5310A_EXIT_MAX_TIMES 4

#define M5310A_RX_DATA_SIZE 			126
#define M5310A_TX_SIZE						16
#define M5310A_SEND_MAX 					126

#define SERVER_IP 			"183.230.40.39"
#define SERVER_PORT			"5683"

typedef enum {
  AT_CMD_NULL = 0,			    	        			//ATģ��������ȴ�����
  AT_CMD_AT,                                //"AT"�������ڲ���MEģ���Ƿ�������ͨ��������
	AT_CMD_CSQ,																//��ѯ�ź�ǿ��
	AT_CMD_CEREG,															//��ѯ���總��״̬
	AT_CMD_CGSN,															//��ѯIMEI��
	AT_CMD_CIMI,
	AT_CMD_MIPLCONF,
	AT_CMD_MIPLADDOBJ,
	AT_CMD_MIPLNOTIFY,
	AT_CMD_MIPLOPEN,
	AT_CMD_MIPLREAD,
	AT_CMD_MIPLCLOSE,
	AT_CMD_MIPLDELOBJ,
	AT_CMD_MIPLDEL,
}AT_Command_t;

typedef enum {
	AT_NULL = 0,
  AT_SET,
  AT_QUERY,
}AT_Type_t;

typedef enum {
  AT_STATE_SEND = 1,		                    //�ѷ�����,ûӦ��
  AT_STATE_BACK,				     								//��Ӧ����
  AT_STATE_TIMEOUT,                         //��Ӧ��ʱ
  AT_STATE_NULL,				     								//û����
  AT_STATE_ERR,				             					//�ظ�����
  AT_STATE_OK,			                     		//�ظ�OK
  AT_STATE_CONNECT_OK,
  AT_STATE_CONNECTING,
  AT_STATE_CONNECT_CLOSE,
  AT_STATE_PPPCONNECT_OK,
  AT_STATE_PPPCONNECT_CLOSE,
}AT_State_t;

typedef struct {
  AT_Command_t	ATCommand;                        //�ȴ��Ĳ�������
  AT_Type_t     ATType;                           //��������  0:������ 1������2����ѯ
  AT_State_t	ATState;                          //��������״̬
  uint32_t      OutTime;                          //״̬��ʱʱ��
  uint32_t      StartTime;                        //����ִ�п�ʼʱ��
  bool          ReturnFlag;                       //�����ѻظ���־
  uint32_t      Option;                           //����صĸ�����Ϣ
}AtCmdInfo_t;

typedef struct
{
	uint8_t			uartBuffer;							//���ڽ��ջ���
	uint8_t			TempReadLen;
	uint8_t 		TxBuffer[M5310A_TX_SIZE];
	uint16_t		RxIndex;							//���ڽ��������±�
	uint8_t 		RxBufferData[M5310A_RX_DATA_SIZE];	//���ڽ�������
	uint16_t		RxDataIndex;						//���յ���AT��Ӧ���ֽ�����
	uint32_t		TxDataLastTime;
	uint32_t 		RxDataLastTime;
	uint8_t			RunStep;
	AtCmdInfo_t		ATInfo;								//ģ��Ӧ������
	uint64_t		imei;								//imei��
	uint64_t		imsi;								//imsi��
	uint32_t		Rssi;
	uint32_t		Ber;
	uint32_t 		Error;
	bool 				RxDataStart;					//ͷ��<CR><LF>��־
	bool				RxIsData;
}M5310A_t;

typedef enum
{
	MIPL_DEBUG_LEVEL_NONE = 0,
	MIPL_DEBUG_LEVEL_RXL,
	MIPL_DEBUG_LEVEL_RXL_RXD,
	MIPL_DEBUG_LEVEL_TXL_TXD,
} MIPL_DEBUG_LEVEL_E;

void MESetAtState(AT_State_t state);

bool Wait_Rec_AT(uint32_t timeout);

void UM5310A_Recv_Enable_Clear(void);

bool Send_At_Command(char *cmd, AT_Command_t cmd_id, AT_Type_t at_type);

void M5310A_RxCallBack(void);

bool Brd_M5310A_Snd_AT(void);

bool Brd_M5310A_AT_ATE(void);

bool Brd_M5310A_AT_CSQ(void);

bool Brd_M5310A_AT_CGSN(void);

bool Brd_M5310A_AT_CEREG(void);

bool Brd_M5310A_AT_CIMI(void);

bool Brd_M5310A_AT_MIPLCONF(void);

bool Brd_M5310A_AT_MIPLADDOBJ(int ref, int16_t objectid, int instanceid);

bool Brd_M5310A_AT_MIPLNOTIFY(int8_t ref, 
															uint16_t objectid,
															int8_t instanceid,
															uint16_t resourceid,
															int8_t valuetype,
															char *value,
															bool flag
															);
															
bool Brd_M5310A_AT_MIPLOPEN(int ref, int16_t timeout);
															
bool Brd_M5310A_AT_MIPLREAD(int8_t ref, 
														int16_t msgid,
														uint16_t objectid,
														int8_t instanceid,
														uint16_t resourceid,
														int8_t valuetype,
														char *value,
														bool flag
														);
														
bool Brd_M5310A_AT_MIPLCLOSE(int ref);
														
bool Brd_M5310A_AT_MMIPLDELOBJ(int8_t ref, int16_t objectid, int8_t instanceid);

bool Brd_M5310A_AT_MMIPLDEL(int8_t ref);
														
void M5310A_Hardware_Init(void);

bool M5310A_Api_Init(void);

bool M5310A_exit(void);

#endif
