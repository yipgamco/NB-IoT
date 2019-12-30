/******************************************************************************
  				成都度云物万物科技有限公司
企业网址：www.doiot.net			企业淘宝：www.doiot.shop
QQ：3453844403	  					微信：do-iot
成都度云万物科技有限公司保留驱动代码版权，你可使用本代码进行移植、修改和开发，
但是原作者以及成都度云万物科技有限公司不对软件做担保和负责
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
