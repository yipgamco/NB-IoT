/******************************************************************************
  				成都度云万物科技有限公司
企业网址：www.doiot.net			企业淘宝：www.doiot.shop
QQ：3453844403	  					微信：do-iot
成都度云万物科技有限公司保留驱动代码版权，你可使用本代码进行移植、修改和开发，
但是原作者以及成都度云万物科技有限公司不对软件做担保和负责
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
  AT_CMD_NULL = 0,			    	        			//AT模块无命令等待返回
  AT_CMD_AT,                                //"AT"命令用于测试ME模块是否与主机通信已连接
	AT_CMD_CSQ,																//查询信号强度
	AT_CMD_CEREG,															//查询网络附着状态
	AT_CMD_CGSN,															//查询IMEI码
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
  AT_STATE_SEND = 1,		                    //已发发送,没应答
  AT_STATE_BACK,				     								//回应命令
  AT_STATE_TIMEOUT,                         //响应超时
  AT_STATE_NULL,				     								//没发送
  AT_STATE_ERR,				             					//回复错误
  AT_STATE_OK,			                     		//回复OK
  AT_STATE_CONNECT_OK,
  AT_STATE_CONNECTING,
  AT_STATE_CONNECT_CLOSE,
  AT_STATE_PPPCONNECT_OK,
  AT_STATE_PPPCONNECT_CLOSE,
}AT_State_t;

typedef struct {
  AT_Command_t	ATCommand;                        //等待的操作命令
  AT_Type_t     ATType;                           //命令类型  0:无命令 1：设置2：查询
  AT_State_t	ATState;                          //操作命令状态
  uint32_t      OutTime;                          //状态超时时间
  uint32_t      StartTime;                        //命令执行开始时间
  bool          ReturnFlag;                       //命令已回复标志
  uint32_t      Option;                           //命令返回的附加信息
}AtCmdInfo_t;

typedef struct
{
	uint8_t			uartBuffer;							//串口接收缓冲
	uint8_t			TempReadLen;
	uint8_t 		TxBuffer[M5310A_TX_SIZE];
	uint16_t		RxIndex;							//串口接收数组下标
	uint8_t 		RxBufferData[M5310A_RX_DATA_SIZE];	//串口接收数组
	uint16_t		RxDataIndex;						//接收到的AT回应的字节数量
	uint32_t		TxDataLastTime;
	uint32_t 		RxDataLastTime;
	uint8_t			RunStep;
	AtCmdInfo_t		ATInfo;								//模块应答内容
	uint64_t		imei;								//imei号
	uint64_t		imsi;								//imsi号
	uint32_t		Rssi;
	uint32_t		Ber;
	uint32_t 		Error;
	bool 				RxDataStart;					//头部<CR><LF>标志
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
