#ifndef __DS18B20_H
#define __DS18B20_H 
#include "sys.h"   
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK MiniSTM32������
//DS18B20��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/12
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

extern void delay_us(uint16_t us);


////IO��������										
#define	DS18B20_DQ_IN HAL_GPIO_ReadPin(DQ_GPIO_Port, DQ_Pin)  //���ݶ˿�	PB0

void DS18B20_IO_OUT(void);
void DS18B20_IO_IN(void);
uint8_t DS18B20_Init(void);			//��ʼ��DS18B20
short DS18B20_Get_Temp(void);	//��ȡ�¶�
void DS18B20_Start(void);		//��ʼ�¶�ת��
void DS18B20_Write_Byte(uint8_t dat);//д��һ���ֽ�
uint8_t DS18B20_Read_Byte(void);		//����һ���ֽ�
uint8_t DS18B20_Read_Bit(void);		//����һ��λ
uint8_t DS18B20_Check(void);			//����Ƿ����DS18B20
void DS18B20_Rst(void);			//��λDS18B20    
#endif















