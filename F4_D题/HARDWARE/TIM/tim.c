/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : tim.c
** @auther  : Mrtutu
** @date    : 2019-08-10
** @describe: 
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/
#include "tim.h"
void TIM7_Int_Init(u16 arr, u16 psc)
{
    /*结构体初始化*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化 GPIO
    
    
    /*使能 TIM7 时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM_TimeBaseInitStructure.TIM_Period = arr; //自动重装载值
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc; //定时器分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    /*初始化定时器TIM7*/
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStructure);
    /*允许定时器 7 更新中断*/
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn; //定时器 7 中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; //抢占优先级 0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02; //响应优先级 1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); // 初始化 NVIC

    TIM_Cmd(TIM7, ENABLE); //使能定时器 7
}


void TIM7_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)==SET) //溢出中断
    {  
        OUT = ~ OUT;
        TIM_ClearITPendingBit(TIM7,TIM_IT_Update); //清除中断标志位
    }
}


/********************************End of File************************************/