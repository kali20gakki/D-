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

    /* ʹ��ʱ�� */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);
    /* GPIO_E  0  2  6*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��

    /* GPIO_C 14*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//ʹ�� SYSCFG ʱ��


    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource6);

    /* ���� �ж��� */
    EXTI_InitStructure.EXTI_Line =  EXTI_Line0 | EXTI_Line2 | EXTI_Line6 ;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
    //EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش���
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�����ش���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ�� LINE11
    EXTI_Init(&EXTI_InitStructure);

    /* �жϳ�ʼ�� LINE0 */
    NVIC_InitStructure.NVIC_IRQChannel =  EXTI0_IRQn; //�ⲿ�ж� 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ� 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//��Ӧ���ȼ� 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);//���� NVIC

    /* �жϳ�ʼ�� LINE2 */
    NVIC_InitStructure.NVIC_IRQChannel =  EXTI2_IRQn; //�ⲿ�ж� 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ� 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//��Ӧ���ȼ� 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);//���� NVIC

    /* �жϳ�ʼ�� LINE6 */
    NVIC_InitStructure.NVIC_IRQChannel =  EXTI9_5_IRQn; //�ⲿ�ж� 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ� 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//��Ӧ���ȼ� 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);//���� NVIC

}




// �˳� PE0
void EXTI0_IRQHandler()
{
    delay_ms(5);
    if(KEY_RST == RESET)
    {
        Mark_Sign.Interface_Mark = M_InputImp; // Ĭ�ϵ�һ��
        Mark_Sign.Task_Mark      = Task_Menu;   // �������˵�
        LCD_Clear(BLACK);
        flagClear = 1;
        flagAmpFreq = 1;
        flag1 = 1;
        Relay_Set(0);
        printf("1000\r\n");
    }
    EXTI_ClearITPendingBit(EXTI_Line0); //��� LINE �ϵ��жϱ�־λ
}


// ȷ�� PE2
void EXTI2_IRQHandler()
{
    delay_ms(5);
    if(KEY_OK == RESET)
    {
        if(Mark_Sign.Task_Mark == Task_Menu) // �ڲ˵��������ȷ��
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
    EXTI_ClearITPendingBit(EXTI_Line2); //��� LINE �ϵ��жϱ�־λ
}


// �Ϲ� PE6
void EXTI9_5_IRQHandler()
{
    delay_ms(5);
    if(KEY_UP == RESET)
    {
        if(Mark_Sign.Task_Mark == Task_Menu)// �ڲ˵���������Ϲ�
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
    EXTI_ClearITPendingBit(EXTI_Line6); //��� LINE �ϵ��жϱ�־λ
}

