/******************************************************************************
  				成都度云万物科技有限公司
企业网址：www.doiot.net			企业淘宝：www.doiot.shop
QQ：3453844403	  					微信：do-iot
成都度云万物科技有限公司保留驱动代码版权，你可使用本代码进行移植、修改和开发，
但是原作者以及成都度云万物科技有限公司不对软件做担保和负责
******************************************************************************/
#include "M5310A.h"

/**************************************************************************
*
*************************************************************************/
typedef void(*WaitEventHandler)(char *pch,AT_Type_t at_type);

void WatiATHandler(char *pch,AT_Type_t at_type);                       //AT_CMD_AT        
void WaitCSQHandler(char *pch,AT_Type_t at_type);											 //AT_CMD_CSQ
void WaitCeregHandler(char *pch,AT_Type_t at_type);										 //AT_CMD_CEREG
void WaitCgsnHandler(char *pch, AT_Type_t at_type);										 //AT_CMD_CGSN
void WaitCimiHandler(char *pch, AT_Type_t at_type);	                   //AT_CMD_CIMI
void WaitMiplconfHandler(char *pch, AT_Type_t at_type);                //AT_CMD_MIPLCONF
void WaitMipladdobjHandler(char *pch, AT_Type_t at_type);	             //AT_CMD_MIPLADDOBJ
void WaitMiplnotifyHandler(char *pch, AT_Type_t at_type);              //AT_CMD_MIPLNOTIFY
void WaitMiplopenHandler(char *pch, AT_Type_t at_type);                //AT_CMD_MIPLOPEN
void WaitMiplreadHandler(char *pch, AT_Type_t at_type);                //AT_CMD_MIPLREAD
void WaitMiplcloseHandler(char *pch, AT_Type_t at_type);               //AT_CMD_MIPLCLOSE
void WaitMipldelobjHandler(char *pch, AT_Type_t at_type);              //AT_CMD_MIPLDELOBJ
void WaitMipldelHandler(char *pch, AT_Type_t at_type);                 //AT_CMD_MIPLDEL

void Active_report(char *pch);

const WaitEventHandler ATWaitEventHandler[] =
{
  NULL, 
  WatiATHandler,
	WaitCSQHandler,
	WaitCeregHandler,
	WaitCgsnHandler,
	WaitCimiHandler,
	WaitMiplconfHandler,
	WaitMipladdobjHandler,
	WaitMiplnotifyHandler,
	WaitMiplopenHandler,
	WaitMiplreadHandler,
	WaitMiplcloseHandler,
	WaitMipldelobjHandler,
	WaitMipldelHandler,
};

extern M5310A_t M5310A;

bool ATAnswerCheck(AT_Command_t cmd,AT_Type_t at_type,char * pch)
{
  if (ATWaitEventHandler[cmd] == NULL)
    return false;
	//当模块返回回显现时立即返回
  if (!strncmp(pch, "AT", 3))
  {
    return true;
  }
  ATWaitEventHandler[cmd](pch,at_type);
	
	return true;
}

void ATHandle(uint8_t *p_ATBuff,uint16_t len)
{
  ATAnswerCheck(M5310A.ATInfo.ATCommand,M5310A.ATInfo.ATType,(char *)p_ATBuff);
	//处理主动上报数据
	Active_report((char *)p_ATBuff);
}

void Active_report(char *pch)
{
//	if(!strncmp(pch, "ERROR:", 6))
//	{
//		sscanf(pch+7, "%u", &M5310A.Error);
		//一些常见的错误
//		switch(M5310A.Error)
//		{
//			case 30:
//				printf("No Network Service\n");
//				break;
//			case 50:
//				printf("Incorrect parameters\n");
//				break;
//			case 311:
//				printf("SIM PIN required\n");
//				break;
//			default:
//				printf("Other error code\n");
//				break;
//		}
//	}
//	if(!strncmp(pch, "+MIPLOPEN:", 10))
//	{
//	}
//	if(!strncmp(pch, "+MIPLOBSERVE:", 13))
//	{
//	}
//	if(!strncmp(pch, "+MIPLREAD:", 10))
//	{
//	}
//	if(!strncmp(pch, "+MIPLDISCOVER:", 14))
//	{
//	}
//	if(!strncmp(pch, "+MIPLCLOSE:", 10))
//	{
//	}
//	if(!strncmp(pch, "+MIPLWRITE:", 10))
//	{
//	}
}
/**************************************************************************
*                        POWER_ON
*************************************************************************/
void WaitPowerOnHandler(char *pch, AT_Type_t at_type)
{
	MESetAtState(AT_STATE_OK);
}

/**************************************************************************
*                        AT_CMD_AT
*************************************************************************/
void WatiATHandler(char *pch,AT_Type_t at_type)
{
  if(!strncmp(pch,ACK_OK,2))
    MESetAtState(AT_STATE_OK);
  else if(!strncmp(pch,"+CME",4))
    MESetAtState(AT_STATE_ERR);
}
/**************************************************************************
*                        //AT_CMD_CSQ,
*************************************************************************/
void WaitCSQHandler(char *pch,AT_Type_t at_type)
{
	static uint8_t cnt = 0;
	if(!strncmp(pch, "+CSQ:", 5) && cnt == 0)
	{
		//当一条指令还有多步返回时不重新去获取/r/n的开头
		M5310A.RxDataStart = true;
		sscanf(pch+5, "%u,%u", &M5310A.Rssi, &M5310A.Ber);
		cnt = 1;
	} else if(!strncmp(pch, ACK_OK, 2) && (cnt == 1))
	{
		MESetAtState(AT_STATE_OK);
		cnt = 0;
	} else if(!strncmp(pch,ACK_ERROR,6))
	{
		MESetAtState(AT_STATE_ERR);
		cnt = 0;
	}
}
/**************************************************************************
*                        //AT_CMD_CEREG,
*************************************************************************/
void WaitCeregHandler(char *pch,AT_Type_t at_type)
{
	static uint8_t cnt = 0;
	if((!strncmp(pch, "+CEREG:0,1", 10) || !strncmp(pch, "+CEREG:0,5", 10))&& cnt == 0)
	{
		//当一条指令还有多步返回时不重新去获取/r/n的开头
		M5310A.RxDataStart = true;
		cnt = 1;
	} else if(!strncmp(pch, ACK_OK, 2) && (cnt == 1))
	{
		MESetAtState(AT_STATE_OK);
		cnt = 0;
	} else if(!strncmp(pch,"+CME ERROR:",11))
	{
		MESetAtState(AT_STATE_ERR);
		cnt = 0;
	}
}

/**************************************************************************
*                        //AT_CMD_CGSN,
*************************************************************************/
void WaitCgsnHandler(char *pch, AT_Type_t at_type)
{
	static uint8_t cnt = 0;

	if(!strncmp(pch, "+CGSN:", 6) && cnt == 0)
	{
		//当一条指令还有多步返回时不重新去获取/r/n的开头
		M5310A.RxDataStart = true;
		sscanf(pch+6, "%llu", &M5310A.imei);
		cnt = 1;
	} else if(!strncmp(pch, ACK_OK, 2) && (cnt == 1))
	{
		MESetAtState(AT_STATE_OK);
		cnt = 0;
	} else if(!strncmp(pch,"+CME ERROR:",11))
	{
		MESetAtState(AT_STATE_ERR);
		cnt = 0;
	}
}
/**************************************************************************
*                        //AT_CMD_CIMI,
*************************************************************************/
void WaitCimiHandler(char *pch, AT_Type_t at_type)
{
	static uint8_t cnt = 0;
	
	if(!strncmp(pch,"+CME ERROR:",11))
	{
		MESetAtState(AT_STATE_ERR);
		cnt = 0;
	} else if(cnt == 0)
	{
		//当一条指令还有多步返回时不重新去获取/r/n的开头
		M5310A.RxDataStart = true;
		sscanf(pch, "%llu", &M5310A.imsi);
		cnt = 1;
	} else if(!strncmp(pch, ACK_OK, 2) && (cnt == 1))
	{
		MESetAtState(AT_STATE_OK);
		cnt = 0;
	}
}
/**************************************************************************
*                        //AT_CMD_MIPLCONF,
*************************************************************************/
void WaitMiplconfHandler(char *pch, AT_Type_t at_type)
{
	static uint8_t cnt = 0;

	if(at_type == AT_SET)
	{
		if (!strncmp(pch,"+MIPLCREATE:",12) && cnt == 0)
		{
			M5310A.RxDataStart = true;
			cnt = 1;			
			//sscanf(pch, "%llu", &M5310A.);
		}
		else if(cnt == 1 && !strncmp(pch,ACK_OK,2))
		{
			MESetAtState(AT_STATE_OK);
			cnt = 0;
		}
		else if(!strncmp(pch,ACK_ERROR,6))
			MESetAtState(AT_STATE_ERR);
	}
}
/**************************************************************************
*                        //AT_CMD_MIPLADDOBJ,
*************************************************************************/
void WaitMipladdobjHandler(char *pch, AT_Type_t at_type)
{
	if(at_type == AT_SET)
	{
		if(!strncmp(pch,ACK_OK,2))
			MESetAtState(AT_STATE_OK);
		else if(!strncmp(pch,ACK_ERROR,6))
			MESetAtState(AT_STATE_ERR);
	}
}
/**************************************************************************
*                        //AT_CMD_MIPLNOTIFY,
*************************************************************************/
void WaitMiplnotifyHandler(char *pch, AT_Type_t at_type)
{
	if(!strncmp(pch,ACK_OK,2))
		MESetAtState(AT_STATE_OK);
	else if(!strncmp(pch,ACK_ERROR,6))
		MESetAtState(AT_STATE_ERR);
}
/**************************************************************************
*                        //AT_CMD_MIPLOPEN,
*************************************************************************/
void WaitMiplopenHandler(char *pch, AT_Type_t at_type)
{
	static uint8_t cnt = 0;
	
	if(cnt == 0 && !strncmp(pch,ACK_OK,2))
	{
		M5310A.RxDataStart = true;
		cnt = 1;
	}
	else if (cnt == 1 && !strncmp(pch,OPEN,9))
	{
		M5310A.RxDataStart = true;
		cnt = 2;
	}
	else if (cnt == 2 && !strncmp(pch,OBS,12))
	{
		M5310A.RxDataStart = true;
		cnt = 3;
	}
	else if (cnt == 3 && !strncmp(pch,DISCOVER,13))
	{
		cnt = 0;
		MESetAtState(AT_STATE_OK);
	}
	else if(!strncmp(pch,ACK_ERROR,6))
	{
		cnt = 0;
		MESetAtState(AT_STATE_ERR);
	}
}
void WaitMiplreadHandler(char *pch, AT_Type_t at_type)
{
	if(!strncmp(pch,ACK_OK,2))
		MESetAtState(AT_STATE_OK);
	else if(!strncmp(pch,ACK_ERROR,6))
		MESetAtState(AT_STATE_ERR);
}
/**************************************************************************
*                        //AT_CMD_MIPLCLOSE,
*************************************************************************/
void WaitMiplcloseHandler(char *pch, AT_Type_t at_type)
{
	static uint8_t cnt = 0;
	printf("%s\n", pch);
	
	if(cnt == 0 &&!strncmp(pch,ACK_OK,2))
	{
		M5310A.RxDataStart = true;
		cnt = 1;		
	} else if (cnt == 1 && !strncmp(pch,"+MIPLCLOSE",10))
	{
		cnt = 0;
		MESetAtState(AT_STATE_OK);
	}
	else if(!strncmp(pch,ACK_ERROR,6))
	{
		cnt = 0;
		MESetAtState(AT_STATE_ERR);
	}
}
/**************************************************************************
*                        //AT_CMD_MIPLDELOBJ,
*************************************************************************/
void WaitMipldelobjHandler(char *pch, AT_Type_t at_type)
{
	if(!strncmp(pch,ACK_OK,2))
		MESetAtState(AT_STATE_OK);
	else if(!strncmp(pch,ACK_ERROR,6))
		MESetAtState(AT_STATE_ERR);
}
/**************************************************************************
*                        //AT_CMD_MIPLDELOBJ,
*************************************************************************/
void WaitMipldelHandler(char *pch, AT_Type_t at_type)
{
	if(!strncmp(pch,ACK_OK,2))
		MESetAtState(AT_STATE_OK);
	else if(!strncmp(pch,ACK_ERROR,6))
		MESetAtState(AT_STATE_ERR);
}
