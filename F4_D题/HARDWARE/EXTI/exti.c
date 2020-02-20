#include "exti.h"
#include "menu.h"
#include "ili9341.h"
#include "relay.h"


volatile u8 flagClear;
volatile u8 flag1;

extern volatile u8 flagAmpFreq;

void KEY_EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* 使能时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);
    /* GPIO_E  0  2  6*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化

    /* GPIO_C 14*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能 SYSCFG 时钟


    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource6);

    /* 配置 中断线 */
    EXTI_InitStructure.EXTI_Line =  EXTI_Line0 | EXTI_Line2 | EXTI_Line6 ;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
    //EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能 LINE11
    EXTI_Init(&EXTI_InitStructure);

    /* 中断初始化 LINE0 */
    NVIC_InitStructure.NVIC_IRQChannel =  EXTI0_IRQn; //外部中断 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//响应优先级 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);//配置 NVIC

    /* 中断初始化 LINE2 */
    NVIC_InitStructure.NVIC_IRQChannel =  EXTI2_IRQn; //外部中断 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//响应优先级 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);//配置 NVIC

    /* 中断初始化 LINE6 */
    NVIC_InitStructure.NVIC_IRQChannel =  EXTI9_5_IRQn; //外部中断 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//响应优先级 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);//配置 NVIC

}




// 退出 PE0
void EXTI0_IRQHandler()
{
    delay_ms(5);
    if(KEY_RST == RESET)
    {
        Mark_Sign.Interface_Mark = M_InputImp; // 默认第一条
        Mark_Sign.Task_Mark      = Task_Menu;   // 返回主菜单
        LCD_Clear(BLACK);
        flagClear = 1;
        flagAmpFreq = 1;
        flag1 = 1;
        Relay_Set(0);
        printf("1000\r\n");
    }
    EXTI_ClearITPendingBit(EXTI_Line0); //清除 LINE 上的中断标志位
}


// 确认 PE2
void EXTI2_IRQHandler()
{
    delay_ms(5);
    if(KEY_OK == RESET)
    {
        if(Mark_Sign.Task_Mark == Task_Menu) // 在菜单界面才能确定
        {
            if(Mark_Sign.Interface_Mark == M_InputImp)
            {
                Mark_Sign.Task_Mark = Task_InputImp;
            }
            else if(Mark_Sign.Interface_Mark == M_OutputImp)
            {
                Mark_Sign.Task_Mark = Task_OutputImp;
            }
            else if(Mark_Sign.Interface_Mark == M_Gain)
            {
                Mark_Sign.Task_Mark = Task_Gain;
            }
            else if(Mark_Sign.Interface_Mark == M_AmpFreq)
            {
                Mark_Sign.Task_Mark = Task_AmpFreq;
            }
            else if(Mark_Sign.Interface_Mark == M_Fault)
            {
                Mark_Sign.Task_Mark = Task_Fault;
            }
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line2); //清除 LINE 上的中断标志位
}


// 上滚 PE6
void EXTI9_5_IRQHandler()
{
    delay_ms(5);
    if(KEY_UP == RESET)
    {
        if(Mark_Sign.Task_Mark == Task_Menu)// 在菜单界面才能上滚
        {
            if(Mark_Sign.Interface_Mark == M_InputImp)
            {
                Mark_Sign.Interface_Mark = M_Fault;
            }
            else if(Mark_Sign.Interface_Mark == M_OutputImp)
            {
                Mark_Sign.Interface_Mark = M_InputImp;
            }
            else if(Mark_Sign.Interface_Mark == M_Gain)
            {
                Mark_Sign.Interface_Mark = M_OutputImp;
            }
            else if(Mark_Sign.Interface_Mark == M_AmpFreq)
            {
                Mark_Sign.Interface_Mark = M_Gain;
            }
            else if(Mark_Sign.Interface_Mark == M_Fault)
            {
                Mark_Sign.Interface_Mark = M_AmpFreq;
            }
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line6); //清除 LINE 上的中断标志位
}

