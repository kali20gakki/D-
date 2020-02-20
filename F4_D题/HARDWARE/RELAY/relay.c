/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : relay.c
** @auther  : Mrtutu
** @date    : 2019-08-09
** @describe:
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/
#include "relay.h"

void Relay_Set(u8 mode);

void Relay_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOB| RCC_AHB1Periph_GPIOD| RCC_AHB1Periph_GPIOC, ENABLE);

    /* PE15 13 11 9  */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化 GPIO
    
    
    /* PD15 13 11 9 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_13 | GPIO_Pin_11 | GPIO_Pin_9 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化 GPIO
    
    /* PB2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化 GPIO
    
    /* PC5 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化 GPIO
    
    Relay_Set(0);
}



/*
* @auther: Mrtutu
* @date  ：2019-08-09
*
* @func  : Relay_Set
* @param : mode: [输入/出]
* @return: None
* @note  : None
*
*    0     全关
*    1     测输入阻抗
*    2     测输出阻抗 无负载
*    3     测输出阻抗 有负载
*    4     测增益
*    5     测幅频特性
*    6     测直流Uo

*
*/
void Relay_Set(u8 mode)
{
    switch(mode)
    {
    /* 全关 */
    case 0:
    {
        Relay1  = 0;
        Relay2  = 0;
        Relay3  = 0;
        Relay4  = 0;
        Relay5  = 0;
        Relay6  = 0;
        Relay7  = 0;
        Relay8  = 0;
        Relay9  = 0;
        Relay10 = 0;
        Relay11 = 0;
        Relay12 = 0;
        Relay13 = 0;
        break;
    }
    /* 测输入阻抗 */
    case 1:
    {
        Relay1  = 0;
        Relay2  = 1;
        Relay3  = 1;
        Relay4  = 1;
        Relay5  = 0;
        Relay6  = 0;
        Relay7  = 0;
        Relay8  = 0;
        Relay9  = 0;
        Relay10 = 0;
        Relay11 = 0;
        Relay12 = 0;
        Relay13 = 0;
        break;
    }

    /* 测输出阻抗 无负载*/
    case 2:
    {
        Relay1  = 1;
        Relay2  = 0;
        Relay3  = 0;
        Relay4  = 0;
        Relay5  = 1;
        Relay6  = 1;
        Relay7  = 0;
        Relay8  = 1;
        Relay9  = 0;
        Relay10 = 0;
        Relay11 = 1;

        break;
    }
    
    /* 测输出阻抗 有负载*/
    case 3:
    {
        Relay1  = 1;
        Relay2  = 0;
        Relay3  = 0;
        Relay4  = 0;
        Relay5  = 1;                     
        Relay6  = 1;
        Relay7  = 0;
        Relay8  = 0;
        Relay9  = 1;
        Relay10 = 1;
        Relay11 = 0;

        break;
    }

    /* 测增益 */
    case 4:
    {
        Relay1  = 1;
        Relay2  = 0;
        Relay3  = 0;
        Relay4  = 0;
        Relay5  = 1;
        Relay6  = 1;
        Relay7  = 0;
        Relay8  = 1;
        Relay9  = 0;
        Relay10 = 0;
        Relay11 = 0;
        Relay12 = 0;
        Relay13 = 0;
        break;
    }

    /* 测幅频特性 */
    case 5:
    {
        Relay1  = 1;
        Relay2  = 0;
        Relay3  = 0;
        Relay4  = 0;
        Relay5  = 1;
        Relay6  = 1;
        Relay7  = 0;
        Relay8  = 1;
        Relay9  = 0;
        Relay10 = 0;
        Relay11 = 0;

        break;
    }
    
    /* 测直流Uo */
    case 6:
    {
        Relay1  = 0;
        Relay2  = 0;
        Relay3  = 0;
        Relay4  = 0;
        Relay5  = 0;
        Relay6  = 0;
        Relay7  = 0;
        Relay8  = 0;
        Relay9  = 0;
        Relay10 = 0;
        Relay11 = 0;
        Relay12 = 0;
        Relay13 = 0;
        break;
    }
    
    /* 输入交流 */
    case 7:
    {
        Relay1  = 0;
        Relay2  = 0;
        Relay3  = 0;
        Relay4  = 0;
        Relay5  = 0;
        Relay6  = 0;
        Relay7  = 0;
        Relay8  = 0;
        Relay9  = 0;
        Relay10 = 0;
        Relay11 = 0;
        Relay12 = 1;
        Relay13 = 1;
        break;
    }
    }
}

/********************************End of File************************************/
