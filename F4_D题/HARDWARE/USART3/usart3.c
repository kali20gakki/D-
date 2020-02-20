/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : usart3.c
** @auther  : Mrtutu
** @date    : 2019-02-17
** @describe: 串口3 底层驱动
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/

#include "usart3.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"

/**************************变量定义******************************/
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN];
__align(8) u8 USART3_TX_BUF[USART3_MAX_SEND_LEN];
u16 USART3_RX_STA = 0; //接收状态标记
/****************************************************************/




/*
* @auther: Mrtutu
* @date  ：2019-02-17
*
* @func  : usart3_Init  串口3初始化
* @param : bound: 波特率
* @return: None
* @note  : TXD：PB10   RXD：PB11
*
*/
void usart3_Init(u32 bound)
{
    /*结构体初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    /*总线使能*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);     //使能GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);    //使能USART3时钟
    /*引脚复用*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3); //GPIOB10复用为USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3); //GPIOB11复用为USART3
    /*USART3 端口初始化*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;              //复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         //速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;            //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;              //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);                    //初始化
    /*串口中断初始化*/
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);            //开启接收中断
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;         //串口1中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级3 数值越小优先级越高
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        //子优先级3,响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           //根据指定的参数初始化VIC寄存器
    /* USART3初始化 */
    USART_InitStructure.USART_BaudRate = bound;               //波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;     //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;        //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
    USART_Init(USART3, &USART_InitStructure);                  //初始化串口3
    USART_Cmd(USART3, ENABLE);                                 //使能串口3
}

void uart3_clearbuff(void)
{
    u16 i;
    for(i = 0; i < USART3_MAX_RECV_LEN; i++)
    {
        USART3_RX_BUF[i] = 0;
    }
}

/*
* @auther: Mrtutu
* @date  ：2019-02-17
*
* @func  : USART3_IRQHandler  串口3接收中断处理
* @param : None
* @return: None
* @note  : None
*
*/
void USART3_IRQHandler(void)
{
    u8 Res;
    /*接收中断 必须以0x0d 0x0a结尾*/
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(USART3); //(USART3->DR) 读取接收到的数据

        if((USART3_RX_STA & 0x8000) == 0) //接收未完成
        {
            if(USART3_RX_STA & 0x4000) //接收到了0x0d
            {
                if(Res != 0x0a)USART3_RX_STA = 0; //接收错误,重新开始
                else USART3_RX_STA |= 0x8000; //接收完成了
            }
            else
            {
                if(Res == 0x0d)USART3_RX_STA |= 0x4000;
                else
                {
                    USART3_RX_BUF[USART3_RX_STA & 0X3FFF] = Res ;
                    USART3_RX_STA++;
                    if(USART3_RX_STA > (USART3_MAX_RECV_LEN - 1))USART3_RX_STA = 0; //接收数据错误,重新开始接收
                }
            }
        }
    }
}



/*
* @auther: Mrtutu
* @date  ：2019-02-17
*
* @func  : usart3_printf   USART3 printf()实现
* @param : fmt: [输入/出]
**			 ...: [输入/出]
* @return: None
* @note  : None
*
*/
void usart3_printf(char* fmt, ...)
{
    u16 i, j;
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char*)USART3_TX_BUF, fmt, ap);
    va_end(ap);
    i = strlen((const char*)USART3_TX_BUF); //此次发送数据的长度
    for(j = 0; j < i; j++) //循环发送数据
    {
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); //等待上次传输完成
        USART_SendData(USART3, (uint8_t)USART3_TX_BUF[j]); 	 //发送数据到串口3
    }

}



/*
*********************************************************************************************************
*	函 数 名: str_to_int
*	功能说明: 将ASCII码字符串转换成整数。 遇到小数点自动越过。
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号，#或0结束。 2014-06-20 修改为非0-9的字符。
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
int str_to_int(char* _pStr)
{
    unsigned char flag;
    char* p;
    int ulInt;
    unsigned char  i;
    unsigned char  ucTemp;

    p = _pStr;
    if(*p == '-')
    {
        flag = 1;	/* 负数 */
        p++;
    }
    else
    {
        flag = 0;
    }

    ulInt = 0;
    for(i = 0; i < 15; i++)
    {
        ucTemp = *p;
        if(ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
        {
            p++;
            ucTemp = *p;
        }
        if((ucTemp >= '0') && (ucTemp <= '9'))
        {
            ulInt = ulInt * 10 + (ucTemp - '0');
            p++;
        }
        else
        {
            break;
        }
    }

    if(flag == 1)
    {
        return -ulInt;
    }
    return ulInt;
}
/********************************End of File************************************/
