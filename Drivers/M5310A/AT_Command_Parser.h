/******************************************************************************
  				�ɶ�����������Ƽ����޹�˾
��ҵ��ַ��www.doiot.net			��ҵ�Ա���www.doiot.shop
QQ��3453844403	  					΢�ţ�do-iot
�ɶ���������Ƽ����޹�˾�������������Ȩ�����ʹ�ñ����������ֲ���޸ĺͿ�����
����ԭ�����Լ��ɶ���������Ƽ����޹�˾��������������͸���
******************************************************************************/
#ifndef _AT_COMMAND_PARSER_H
#define _AT_COMMAND_PARSER_H

#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include <stdbool.h>

#define OBS "+MIPLOBSERVE"
#define DISCOVER "+MIPLDISCOVER"
#define OPEN "+MIPLOPEN"
#define ACK_OK "OK"
#define ACK_ERROR "ERROR"

void ATHandle(uint8_t *p_ATBuff,uint16_t len);

#endif
