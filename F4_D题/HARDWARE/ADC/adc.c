/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : adc.c
** @auther  : Mrtutu
** @date    : 2019-07-07
** @describe:
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/

#include "adc.h"

volatile u8 flagDMA = 0; // DMA 传输完成标志
volatile uint16_t ADC1ConvertedValue[NPT]; // ADC1 DMA目标数组
volatile uint16_t ADC3ConvertedValue[NPT]; // ADC3 DMA目标数组

extern volatile float ADC1__FFT_InputBuff[2 * NPT]; 
extern volatile float ADC3__FFT_InputBuff[2 * NPT];



void TIM1_Trig_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	/* 使能TIM1时钟 */

    TIM_Cmd(TIM1, DISABLE);
    TIM_SetCounter(TIM1, 0);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    /* ARR自动重装载寄存器周期的值(定时时间）到设置频率后产生个更新或者中断. */
    TIM_TimeBaseStructure.TIM_Period =  arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;	/* 不分频 */
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;		/* TIM1 和 TIM8 必须设置 */
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /**************ADC1 ADC3的触发***********************************************/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (TIM_TimeBaseStructure.TIM_Period - 1) / 2;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;		/* only for TIM1 and TIM8. 此处和正相引脚不同 */
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;			/* only for TIM1 and TIM8. */
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;		/* only for TIM1 and TIM8. */
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;		/* only for TIM1 and TIM8. */
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);	/* 使能PWM 输出, 不是输出到GPIO */
}

void ADC2_Int_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE); //使能 ADC2 时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC2, ENABLE); //ADC1 复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC2, DISABLE); //复位结束

    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟 5 个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA 失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频 4 分频。
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12 位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    //禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;//1 个转换在规则序列中
    ADC_Init(ADC2, &ADC_InitStructure);//ADC 初始化

    ADC_Cmd(ADC2, ENABLE);//开启 AD 转换器
}


void ADC_Dual_Sync_Init(void)
{
    ADC_InitTypeDef       ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    DMA_InitTypeDef       DMA_InitStructure;
    GPIO_InitTypeDef      GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    /* 使能 ADC1, ADC2, DMA2 和 GPIO 时钟 ****************************************/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC3, ENABLE);

    /* DMA2 Stream1 channel2 配置用于ADC3 **************************************/
    DMA_DeInit(DMA2_Stream1);
    DMA_InitStructure.DMA_Channel = DMA_Channel_2;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC3->DR); //外设基地址
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC3ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = NPT;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	// 无需循环模式 DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream1, &DMA_InitStructure);
    DMA_Cmd(DMA2_Stream1, ENABLE);

    /* DMA2 Stream0 channel0 配置用于ADC1 **************************************/
    DMA_DeInit(DMA2_Stream0);
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC1->DR); //外设基地址
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC1ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = NPT;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	// 无需循环模式 DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC); //清除中断标志
    DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE); //传输完成中断
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
    DMA_Cmd(DMA2_Stream0, ENABLE);


    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;  //DMA2_Stream0中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化NVIC寄存器

//    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;  //DMA2_Stream1中断
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级1
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;       //子优先级1
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道使能
//    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化NVIC寄存器

    /* 配置ADC引脚为模拟输入模式******************************/
    // ADC1 CH1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ADC3 CH10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    /* ADC公共部分初始化**********************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInit(&ADC_CommonInitStructure);


    /*ADC1的配置******************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;

    /* ADC1 规则通道配置 -------------------------------------------------------*/
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);

    /* 使能 ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* 使能DMA请求 (多ADC模式) --------------------------------------------------*/
    //ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

    /* 使能 ADC1 --------------------------------------------------------------*/
    ADC_Cmd(ADC1, ENABLE);


    /*ADC3的配置*****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;

    /* ADC3 规则通道配置 */
    ADC_Init(ADC3, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 1, ADC_SampleTime_3Cycles);

    /* 使能 ADC3 DMA */
    ADC_DMACmd(ADC3, ENABLE);


    /* 使能DMA请求 (多ADC模式) --------------------------------------------------*/
    //ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

    /* 使能 ADC3 --------------------------------------------------------------*/
    ADC_Cmd(ADC3, ENABLE);

}


// 停止ADC 和 DMA
void ADC_DMA_Pause(void)
{
    TIM_Cmd(TIM1, DISABLE);

    ADC_DMACmd(ADC1, DISABLE);	/* 这句话必须有，否则无法启动下次DMA */
    ADC_DMACmd(ADC3, DISABLE);
}


// 继续 ADC和 DMA
void ADC_DMA_Continue(void)
{
    TIM_Cmd(TIM1, ENABLE);
    TIM_SetCounter(TIM1, 0);

    ADC_DMACmd(ADC1, ENABLE);	/* 这句话必须有，否则无法启动下次DMA */
    ADC_DMACmd(ADC3, ENABLE);
}


void DMA2_Stream0_IRQHandler(void)
{
    u16 i;
    if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))  //判断DMA传输完成中断
    {
        ADC_DMA_Pause();

        for(i = 0; i < NPT; i++)
        {
            ADC1__FFT_InputBuff[2 * i] = (float)ADC1ConvertedValue[i]; // 实部
            ADC1__FFT_InputBuff[2 * i + 1] = 0; // 虚部为零

            ADC3__FFT_InputBuff[2 * i] = (float)ADC3ConvertedValue[i]; // 实部
            ADC3__FFT_InputBuff[2 * i + 1] = 0; // 虚部为零
        }

        flagDMA = 1;
        DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0); // 清除标志位
    }

}

// CH: ADC_Channel_0
// CH: ADC_Channel_1
// CH: ADC_Channel_3
u16 Get_ADCValue(u8 ch)
{
    //设置指定 ADC 的规则组通道，一个序列，采样时间
    ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_480Cycles);
    ADC_SoftwareStartConv(ADC2); //使能指定的 ADC1 的软件转换启动功能
    while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC)); //等待转换结束
    return ADC_GetConversionValue(ADC2);
}



void Sort_tab(u16 tab[], u16 length)
{
    u16 i;
    u8 flag = 1;
    u16 temp = 0;

    // 排序
    while(flag)
    {
        flag = 0;
        for(i = 0; i < length - 1; i++)
        {
            if(tab[i] > tab[i + 1])
            {
                temp       = tab[i];
                tab[i]     = tab[i + 1];
                tab[i + 1] = temp;
                flag       = 1;
            }
        }
    }
}


/*
* @auther: Mrtutu
* @date  ：2019-06-26
*
* @func  : ADC_GetSampleFliter
* @param : N: 采样个数          400(默认)
**		   X: 去极大极小值个数  80(默认)
* @return: None
* @note  : None
*
*/
u16 ADC_GetSampleFliter(u8 ch, u16 N, u16 X)
{
    u32 adc_sum;
    u16 adc_sample[400]; // 采样数目
    u16 i;

    /** 采样 **/
    for(i = 0; i < N; i++)
    {
        adc_sample[i] = Get_ADCValue(ch);
    }

    // 排序
    Sort_tab(adc_sample, 400);

    // 去噪相加
    for(i = X / 2; i < (N - X / 2); i++)
    {
        adc_sum += adc_sample[i];
    }

    // 均值
    adc_sum /= N - X;

    return adc_sum;

}




/********************************End of File************************************/