/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : usart3.h
** @auther  : Mrtutu
** @date    : 2019-02-17
** @describe: 
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/
 
#ifndef __USART3_H_
#define __USART3_H_
#include "sys.h"
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#define USART3_MAX_RECV_LEN 255
#define USART3_MAX_SEND_LEN 255

extern u8 USART3_RX_BUF[USART3_MAX_RECV_LEN];
extern u8 USART3_TX_BUF[USART3_MAX_SEND_LEN];
extern u16 USART3_RX_STA; //接收状态标记 
void uart3_init(u32 bound);

#endif

/********************************End of File************************************/