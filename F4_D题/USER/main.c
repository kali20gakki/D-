#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "ili9341.h"
#include "menu.h"
#include "math.h"
#include "exti.h"
#include "adc.h"
#include "usart3.h"
#include "relay.h"


float K = 5.05;  // 电压衰减倍数

/*系统参数********************************************/

volatile static u32 Normal_IN_Zx;   // 输入阻抗
volatile static u32 Normal_OUT_Zx;  // 输出阻抗
volatile static u32 Normal_Gain;    // 放大倍数
volatile static u32 Normal_FH;      // 上限频率

/*系统参数********************************************/

extern volatile u8 flagClear;
extern volatile u8 flagDMA;
extern volatile u8 flag1;

volatile u8 flagAmpFreq = 1;


/***参数定义******************************************************/
u8 buff1[32];
u8 buff2[32];
u8 buff3[16];
u8 buff4[16];

u16 ADC1_Offest;
u16 ADC3_Offest;
u16 ADC1_Amplitude = 0;
u16 ADC3_Amplitude = 0;
u16 ADC1_MAX_INDEX;
u16 ADC3_MAX_INDEX;
u16 ADC1_Vpp;
u16 ADC3_Vpp;

volatile float ADC1__FFT_InputBuff[2 * NPT];
volatile float ADC3__FFT_InputBuff[2 * NPT];

volatile float ADC1__FFT_OutputBuff[2 * NPT];
volatile float ADC3__FFT_OutputBuff[2 * NPT];

volatile u16 ADC1_MagBuff[NPT / 2];
volatile u16 ADC3_MagBuff[NPT / 2];
/**********************************************************/

/***函数定义************************************************/
void Task_inputImp(void);
void Task_outputImp(void);
void Task_gain(void);
void Task_ampFreq(void);
void Task_fault(void);
void LCD_ShowFrame(void);
float kalman_filter(uint16_t ADC_Value);
int str_to_int(char* _pStr);
u8 Uo_is_HalfWave(void);
float Get_DC_Uo(void);
float Get_DC_Uo_bug(void);
u32 Get_IN_Zx(void);
void State_Check(void);
u16 Get_Gain(void);
u16 Get_Gainx(void);
u32 Get_Out_Zx(void);
/**********************************************************/





int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init(168);
    uart_init(115200);
    ADC2_Int_Init();
    Relay_Init();
    KEY_EXTI_Init();
    TIM7_Int_Init(25 - 1, 24 - 1);
    LCDx_Init();
    LCD_Clear(BLACK);
    BACK_COLOR = BLACK;
    /* 菜单结构体初始化 */
    Mark_Sign.Interface_Mark = M_InputImp;
    Mark_Sign.Task_Mark      = Task_Menu;
    flagClear = 1;


    while(1)
    {
        LCD_Show_R1_Open();
        delay_ms(1000);
        LCD_Show_R2_Open();
        delay_ms(1000);
        LCD_Show_R3_Open();
        delay_ms(1000);
        LCD_Show_R4_Open();
        delay_ms(1000);
        
        LCD_Show_R1_Short();
        delay_ms(1000);
        LCD_Show_R2_Short();
        delay_ms(1000);
         LCD_Show_R3_Short();
        delay_ms(1000);
        LCD_Show_R4_Short();
        delay_ms(1000);
        
        LCD_Show_C1_Open();
        delay_ms(1000);
        LCD_Show_C2_Open();
        delay_ms(1000);
        LCD_Show_C3_Open();
        delay_ms(1000);
        
        
        LCD_Show_C1_x2();
        delay_ms(1000);
        LCD_Show_C2_x2();
        delay_ms(1000);
        LCD_Show_C3_x2();
        delay_ms(1000);
        
        LCD_Show_Normal();
        delay_ms(1000);
    }
    while(1)
    {
        switch(Mark_Sign.Task_Mark)
        {
        /*菜单界面下******************************************************************/
        case Task_Menu:
        {
            switch(Mark_Sign.Interface_Mark)
            {
            /* 输入阻抗界面 */
            case M_InputImp:
            {
                if(flagClear == 1) // 保证每次清屏没有残留
                {
                    LCD_Clear(BLACK);
                    flagClear = 0;
                }
                State_Check();
                Menu_InputImp();
                break;
            }
            /* 输出阻抗界面 */
            case M_OutputImp:
            {
                State_Check();
                Menu_OutputImp();
                break;
            }
            /* 增益界面 */
            case M_Gain:
            {
                State_Check();
                Menu_Gain();
                break;
            }
            /* 幅频特性界面 */
            case M_AmpFreq:
            {
                State_Check();
                Menu_AmpFreq();
                break;
            }
            /* 故障判断界面 */
            case M_Fault:
            {
                State_Check();
                Menu_Fault();
                break;
            }
            }
            break; // 一定要break;
        }

        /*输入阻抗********************************************************************/
        case Task_InputImp:
        {
            //printf("40\r\n");
            Task_inputImp();
            break;
        }

        /*输出阻抗********************************************************************/
        case Task_OutputImp:
        {
            Task_outputImp();
            break;
        }

        /*增益************************************************************************/
        case Task_Gain:
        {
            Task_gain();
            break;
        }

        /*幅频特性********************************************************************/
        case Task_AmpFreq:
        {
            Task_ampFreq();
            break;
        }

        /*故障测试********************************************************************/
        case Task_Fault:
        {
            Task_fault();
            break;
        }
        }

    }
}





/*
* @auther: Mrtutu
* @date  ：2019-08-07
*
* @func  : Task_inputImp
* @param : None
* @return: None
* @note  : None
* 输入阻抗测试
*/
void Task_inputImp(void)
{
    u8 cnt;
    float vol;
    float Zx;
    float temp;
    u16 adcx;
    Relay_Set(1);
    u8 buff_adc[32];
    LCD_Clear(BLACK);

    while(Mark_Sign.Task_Mark == Task_InputImp)
    {

        adcx = ADC_GetSampleFliter(ADC_Channel_0, 400, 80);

        vol = adcx * 3300 / 4095;
        temp = (vol + 1) / 10.845f;
        Zx = 10000 * (103.62f - temp) / temp;

        if(cnt > 10)
        {
            cnt = 0;
            LCD_Clear(BLACK);
        }
        sprintf(buff_adc, "IN_Zx = %.0f Ohm", Zx);
        LCD_ShowString(75, 40, buff_adc, RED);
        sprintf(buff_adc, "Vol   = %.0f mV", vol);
        LCD_ShowString(75, 80, buff_adc, RED);
        vol = vol * 1.0373f - 161.72f;
        sprintf(buff_adc, "Vpp   = %.0f mV", vol);
        LCD_ShowString(75, 120, buff_adc, RED);
        delay_ms(300);
        cnt += 1;

        Normal_IN_Zx = (u32)Zx;
    }
}




/*
* @auther: Mrtutu
* @date  ：2019-08-07
*
* @func  : Task_outputImp
* @param : None
* @return: None
* @note  : None
* 输出阻抗测试
*/
void Task_outputImp(void)
{
    u8 cnt;
    float Zx;
    u16 adcx1;
    u16 adcx2;
    float vol1;
    float vol2;
    u8 buff_adc[32];

    LCD_Clear(BLACK);
    while(Mark_Sign.Task_Mark == Task_OutputImp)
    {
        Relay_Set(2); // 无负载
        delay_ms(50);
        adcx1 = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
        vol1 = adcx1 * 3300 / 4095;

        Relay_Set(3); // 有负载
        delay_ms(50);
        adcx2 = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
        vol2 = adcx2 * 3300 / 4095;


        Zx = (float)(1597 * (vol1 - vol2) / vol2);

        if(cnt > 10)
        {
            cnt = 0;
            LCD_Clear(BLACK);
        }
        sprintf(buff_adc, "Out_Zx = %.0f Ohm", Zx);
        LCD_ShowString(75, 40, buff_adc, RED);
        sprintf(buff_adc, "Vol1   = %.0f mV", vol1);
        LCD_ShowString(75, 80, buff_adc, RED);
        sprintf(buff_adc, "Vol2   = %.0f mV", vol2);
        LCD_ShowString(75, 120, buff_adc, RED);

        cnt += 1;
        delay_ms(200);

        Normal_OUT_Zx = (u32)Zx;
    }
}

u32 Task_outputImpx(void)
{
    u8 cnt;
    float Zx;
    u16 adcx1;
    u16 adcx2;
    float vol1;
    float vol2;
    u8 buff_adc[32];

    LCD_Clear(BLACK);
    while(Mark_Sign.Task_Mark == Task_OutputImp)
    {
        Relay_Set(2); // 无负载
        delay_ms(50);
        adcx1 = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
        vol1 = adcx1 * 3300 / 4095;

        Relay_Set(3); // 有负载
        delay_ms(50);
        adcx2 = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
        vol2 = adcx2 * 3300 / 4095;


        Zx = (float)(1597 * (vol1 - vol2) / vol2);

        if(cnt > 10)
        {
            cnt = 0;
            LCD_Clear(BLACK);
        }
        sprintf(buff_adc, "Out_Zx = %.0f Ohm", Zx);
        LCD_ShowString(75, 40, buff_adc, RED);
        sprintf(buff_adc, "Vol1   = %.0f mV", vol1);
        LCD_ShowString(75, 80, buff_adc, RED);
        sprintf(buff_adc, "Vol2   = %.0f mV", vol2);
        LCD_ShowString(75, 120, buff_adc, RED);

        cnt += 1;
        delay_ms(200);

        Normal_OUT_Zx = (u32)Zx;
    }
}



/*
* @auther: Mrtutu
* @date  ：2019-08-07
*
* @func  : Task_gain
* @param : None
* @return: None
* @note  : None
* 增益测试  AD
*/
void Task_gain(void)
{
    u16 adcx;
    float Vin = 7.52;
    float vol;
    float gain;
    u8 BUFF[32];
    Relay_Set(4);
    LCD_Clear(BLACK);

    while(Mark_Sign.Task_Mark == Task_Gain)
    {

//        /* 用二极管初测 */
//        Relay_Set(4);
//        delay_ms(10);
//        adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
//        vol = adcx * 3300 / 4095;
//        vol = vol * 1.0373f - 161.72f;
//        vol = vol * 0.7071f;

//        if(vol < 400)// 小值用 637
//        {
//            Relay_Set(2);
//            delay_ms(10);
//            adcx = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
//            vol = adcx * 3300 / 4095;
//
//            sprintf(BUFF, "Vol = %.0f mV", vol);
//            LCD_ShowString(75, 40, BUFF, RED);
//            gain = (float)(vol / Vin);
//            sprintf(BUFF, "Gain = %.0f ", gain);
//            LCD_ShowString(75, 80, BUFF, RED);
//
//            Normal_Gain = (u32)gain;
//        }
//        else         // 大值用二极管
//        {
//            Relay_Set(4);
//            delay_ms(10);
//            adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
//            vol = adcx * 3300 / 4095;
//            vol = vol * 1.0373f - 161.72f;
//            vol = vol * 0.7071f;
//
//            sprintf(BUFF, "Vol = %.0f mV", vol);
//            LCD_ShowString(75, 40, BUFF, RED);
//            gain = (float)(vol / Vin);
//            sprintf(BUFF, "Gain = %.0f ", gain);
//            LCD_ShowString(75, 80, BUFF, RED);
//
//            Normal_Gain = (u32)gain;
//        }

/*modify
        adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
        vol = adcx * 3300 / 4095;
        vol = vol * 1.0373f - 161.72f;
        vol = vol * 0.7071f;

        sprintf(BUFF, "Vol = %.0f mV", vol);
        LCD_ShowString(75, 40, BUFF, RED);
        gain = (float)(vol / Vin);
        sprintf(BUFF, "Gain = %.0f ", gain);
        LCD_ShowString(75, 80, BUFF, RED);

        Normal_Gain = (u32)gain;
        delay_ms(200);
*/
        gain = Get_Gainx();
        
        sprintf(BUFF, "Gain = %6.1f ", gain);
        LCD_ShowString(75, 80, BUFF, RED);
        
        delay_ms(200);
    }
}



void  Task_ampFreq(void)
{
    u16 n;
    float temp;
    u8 BUFF[32];
    u16 ADC_MAX;          // 最大值
    u16 x[10] = {20,31,42,53,64,75,86,97,108,119};
    u16 ADC_Value[20];    // 找最大值数组
    u16 ADC_Temp[400];
    u16 ADC_Draw[280];
    u16 adcx;
    u16 Value_3db;        // -3db值
    u16 last_value;
    u32 Freq = 0;         // 开始频率
    u32 Start_Freq = 0;   // 开始频率
    u16 step;             // 间隔
    u8  Index_3db = 0;    // 3db下标
    u8  Index_max = 0;
    u32 Freq_in_max = 0;
    u32 FH;              // 极限频率
    float vol;
    Relay_Set(5);
    LCD_Clear(BLACK);

    printf("1000\r\n");
    while(Mark_Sign.Task_Mark == Task_AmpFreq)
    {
        if(flagAmpFreq == 1 && Mark_Sign.Task_Mark == Task_AmpFreq)
        {
            LCD_ShowFrame();
            /*中等 频谱宽度*/
            Start_Freq = 1000;
            step = 2500;
            for(n = 0; n < 20; n++)
            {
                Freq = Start_Freq + n * step;
                printf("%d\r\n", Freq);
                delay_ms(50);
                adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
                //ADC_Value[n] = (u16)(adcx * 3300 / 4095);
                vol = adcx * 3300 / 4095;
                ADC_Value[n] = (u16)(vol * 1.0373f - 161.72f);
            }

            for(n = 0; n < 20; n++)
            {
                if(ADC_MAX < ADC_Value[n])
                {
                    ADC_MAX = ADC_Value[n];
                    Index_max = n;
                }
            }
            // 最大值对应的频率
            Freq_in_max = Start_Freq + (Index_max + 1) * step;

            sprintf(BUFF, "MAX=%d", ADC_MAX);
            LCD_ShowString(45, 0, BUFF, RED);

            sprintf(BUFF, "F=%d", Freq_in_max);
            LCD_ShowString(120, 0, BUFF, RED);

            Value_3db = (u16)(ADC_MAX * 0.7071f);
            sprintf(BUFF, "3db=%d", Value_3db);
            LCD_ShowString(45, 20, BUFF, RED);


            Start_Freq = Freq_in_max;
            step = 500;

            // 采样
            for(n = 0; n < 400 - 1; n++)
            {
                Freq = Start_Freq + n * step;
                printf("%d\r\n", Freq);
                delay_ms(10);
                adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
                //ADC_Temp[n] = (u16)(adcx * 3300 / 4095);
                vol = adcx * 3300 / 4095;
                ADC_Temp[n] = (u16)(u16)(vol * 1.0373f - 161.72f);
            }

            // 找离3db 最近的点
            temp = (float)fabs(ADC_Temp[0] - Value_3db);
            for(n = 1; n < 400 - 1; n++)
            {
                if((float)fabs(Value_3db - ADC_Temp[n]) < temp)
                {
                    temp = (float)fabs(Value_3db - ADC_Temp[n]);
                    Index_3db = n;
                }
            }

            sprintf(BUFF, "V=%d", ADC_Temp[Index_3db]);
            LCD_ShowString(120, 20, BUFF, RED);

            FH = (u32)(Start_Freq + Index_3db * step);


            /*采点画图*/
//            for(n = 0; n<280-1; n++)
//            {
//                if(n < 50) //低频
//                {
//                    step = 400;
//                    Freq = Start_Freq + n * step;
//                    printf("%d\r\n", Freq);
//                    delay_ms(5);
//                    adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
//                    vol = adcx * 3300 / 4095;
//                    ADC_Draw[n] = (u16)(u16)(vol * 1.0373f - 161.72f);
//                }
//                else
//                {
//                    step = 2000;
//                    Freq +=  step;
//                    printf("%d\r\n", Freq);
//                    delay_ms(5);
//                    adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
//                    vol = adcx * 3300 / 4095;
//                    ADC_Draw[n] = (u16)(u16)(vol * 1.0373f - 161.72f);
//                }
//            }
            Start_Freq = 1000;
            step = 550;
            for(n = 0; n < 280 - 1; n++)
            {
                Freq = Start_Freq + n * step;
                printf("%d\r\n", Freq);
                delay_ms(5);
                adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
                //ADC_Draw[n] = (u16)(adcx * 3300 / 4095);
                vol = adcx * 3300 / 4095;
                ADC_Draw[n] = (u16)(u16)(vol * 1.0373f - 161.72f);
            }
            // 归一化
            for(n = 0; n < 280 - 1; n++)
            {
                ADC_Draw[n] = (u16)(ADC_Draw[n] * 200 / 3300); // 归一化
            }

//            /* 画图 */
//            for(n = 0; n < 10; n++) // 低频
//            {
//                LCD_DrawLine(x[n], LCD_H - (ADC_Draw[n] + 20), x[n+1], LCD_H - (ADC_Draw[n + 1] + 20), RED);
//            }
//            for(n = 0; n < 170; n++)
//            {
//                LCD_DrawLine((119 + n), LCD_H - (ADC_Draw[n+10] + 20), (120 + n), LCD_H - (ADC_Draw[n + 11] + 20), RED);
//            }
//            last_value = 21;
            for(n = 1; n < 280 - 2; n++)
            {
  
                // x轴加20   y轴倒置减20
                LCD_DrawLine((n + 20), LCD_H - (ADC_Draw[n] + 20), (n + 21), LCD_H - (ADC_Draw[n + 1] + 20), RED);
 
            }

            if(FH > 100000 && FH < 120000)
            {
                FH += 25000;
            }
            sprintf(BUFF, "FH = %d Hz", FH);
            LCD_ShowString(200, 0, BUFF, RED);
            flagAmpFreq = 0;

            Normal_FH = (u32)FH;
        }
    }

}





/*
* @auther: Mrtutu
* @date  ：2019-08-07
*
* @func  : Task_fault
* @param : None
* @return: None
* @note  : None
* 故障测试
*/
void Task_fault(void)
{
    float Uo;
    u32 IN_Zx, OUT_Zx;
    u16 Gain;
    u8 BUFF[16];
    u16 Gain_300K;
    float Vcc = 11.98;  // 电源电压
    LCD_Clear(BLACK);
    LCD_ShowString(75, 40, "Start ... ", RED);

    while(Mark_Sign.Task_Mark == Task_Fault)
    {
        /*1. 断开信号 测 Uo*/
        printf("1000\r\n");
        Relay_Set(6);
        delay_ms(20);
        Uo = Get_DC_Uo_bug();
        
    sprintf(BUFF, "Uo = %6.1fV", Uo);
    LCD_ShowString(75, 80, BUFF, RED);
        
        /*2. 根据Uo判断电路状态*/
        
        /* 正常 ************************************************/
        if(Uo >= 5.0 && Uo <= 8.4)
        {
            printf("50\r\n");
            Relay_Set(1);
            delay_ms(200);
            IN_Zx = Get_IN_Zx();
            
            printf("100000\r\n");
            delay_ms(200);
            OUT_Zx = Get_Out_Zx();
            
            
            if(IN_Zx > 10000000)
                LCD_ShowString(75, 40, "C1   Open", RED);
            else if(IN_Zx <= 150000 && IN_Zx > 70000)
                LCD_ShowString(75, 40, "C2   Open", RED);
            else if(IN_Zx <= 61800 && IN_Zx > 60000)
                LCD_ShowString(75, 40, "C1   x2", RED);
            else if(IN_Zx <= 55000 && IN_Zx > 50000)
                LCD_ShowString(75, 40, "C2   x2", RED);
            else if(IN_Zx <= 80000 && IN_Zx > 61900)
            {                
                if(OUT_Zx > 1200)
                    LCD_ShowString(75, 40, "C3   Open", RED);
                else if(OUT_Zx <= 700 && OUT_Zx > 500)
                    LCD_ShowString(75, 40, "C3     x2", RED);
                else if(OUT_Zx <= 1200 && OUT_Zx > 800)
                    LCD_ShowString(75, 40, "Work     ", RED);
            }
        }
        
        /* 饱和 ***********************************************/
        else if(Uo <= 4.5 && Uo > 1)
        {
            LCD_ShowString(75, 40, "R2   Open", RED); //OK
        }
        else if(Uo < 0.3 && Uo > 0.1)
        {
            LCD_ShowString(75, 40, "R3   Open", RED);
        }
        else if(Uo < 0.1)
        {
            LCD_ShowString(75, 40, "R4   Short", RED);
        }
        
        /* 正偏 ************************************************/
        else if(fabs(Uo - 11.2) < 0.1)
        {
            //R1 短路
            LCD_ShowString(75, 40, "R1  Short ", RED); //OK
        }
        
        /* 截止 ************************************************/
        else if(Uo >= 11.4)
        {
            /* 加交流信号(高幅度) 测Uo */
            Relay_Set(7);

            delay_ms(200);
            Uo = Get_DC_Uo(); //测电压

            if(Uo < 11.4)//Uo < 11.4
            {
                //R1 开路
                LCD_ShowString(75, 40, "R1   Open ", RED); //OK
            }
            else// 测输入阻抗
            {
                Relay_Set(1);
                delay_ms(200);
                IN_Zx = Get_IN_Zx();

                if(IN_Zx > 8000 && IN_Zx < 14000)
                {
                    //R4 开
                    LCD_ShowString(75, 40, "R4   Open ", RED); // OK
                }
                else if(IN_Zx > 1000 && IN_Zx < 3000)
                {
                    //R3 短路
                    LCD_ShowString(75, 40, "R3   Short", RED); //OK
                }
                else if(IN_Zx < 50)
                {
                    //R2 短
                    LCD_ShowString(75, 40, "R2  Short ", RED); //ok
                }
            }
        }




    sprintf(BUFF, "In = %8d Ohm", IN_Zx);
    LCD_ShowString(75, 120, BUFF, RED);
    sprintf(BUFF, "Out = %8d Ohm", OUT_Zx);
    LCD_ShowString(75, 160, BUFF, RED);        
            

    }
}


void State_Check(void)
{
    float vol;
    u16 adcx;
    u8 BUFF[16];
    Relay_Set(6);

    adcx = ADC_GetSampleFliter(ADC_Channel_2, 400, 80);
    vol = (float)(adcx * 3.3 / 4095);
    vol = (float)vol * K; // 实际Uo电压
    sprintf(BUFF, "%.2f  ", vol);
    LCD_ShowString(0, 20, BUFF, RED);
    if(vol > 5.2 && vol < 8.5) // 正常放大状态
    {
        LCD_ShowString(0, 0, "OK   ", RED);
        delay_ms(50);
    }
    else
    {
        LCD_ShowString(0, 0, "Error", RED);
        delay_ms(50);
    }
}


// 测量增益
u16 Get_Gain(void)
{
    u16 adcx;
    float vol;
    float gain;
    Relay_Set(4);

    delay_ms(200);

    adcx = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
    vol = adcx * 3300 / 4095;
    vol = vol * 1.0373f - 161.72f;
    vol = vol * 0.7071f;


    gain = (float)(vol / 7.52);

    return (u16)gain;
}

u16 Get_Gainx(void)
{
    float vol;
    float gain;
    u16 ADC_3V, ADC_400mV;
    
    delay_ms(200);
    
    ADC_3V = ADC_GetSampleFliter(ADC_Channel_1, 400, 80);
    vol = ADC_3V * 3300 / 4095;
    vol = vol * 1.0373f - 161.72f;
    vol = vol * 0.7071f;
    
    if(vol<400)
    {
        Relay11 = 1;
        
        delay_ms(200);
        ADC_400mV = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
        vol = (ADC_400mV * 3300 / 4095);
        
        Relay11 = 0;
    }

    gain = (float)(vol / 7.52)+1;
    return (u16)gain;
}



// 判断是否是半波
// 1: 半波
// 0：直流
u8 Uo_is_HalfWave(void)
{
    u8 i;
    u16 adc_temp[6];
    /*判断是否有波形*/

    for(i = 0; i < 6; i++)
    {
        adc_temp[i] = ADC_GetSampleFliter(ADC_Channel_2, 400, 80);
    }

    if(abs(adc_temp[0] - adc_temp[1]) > 50 && abs(adc_temp[2] - adc_temp[3]) > 50 && abs(adc_temp[4] - adc_temp[5]) > 50)
    {
        return 1;
    }
    else return 0;

}


// 测直流Uo
float Get_DC_Uo(void)
{
    u16 adcx;
    float vol;
    Relay_Set(7);

    delay_ms(200);
    // 使用AD CH2
    adcx = ADC_GetSampleFliter(ADC_Channel_2, 400, 80);
    vol = adcx * 3.3 / 4095;
    vol = (float)vol * K; // 实际Uo电压
    return (float)vol;
}

// 测直流Uo
float Get_DC_Uo_bug(void)
{
    u16 adcx;
    float vol;

    delay_ms(200);
    // 使用AD CH2
    adcx = ADC_GetSampleFliter(ADC_Channel_2, 400, 80);
    vol = adcx * 3.3 / 4095;
    vol = (float)vol * K; // 实际Uo电压
    return (float)vol;
}


// 测输入阻抗
u32 Get_IN_Zx(void)
{
    float vol;
    float Zx;
    float temp;
    u16 adcx;

    /* 挡位选择 */
    Relay_Set(1);

    delay_ms(200);
    /* 采样计算 */
    adcx = ADC_GetSampleFliter(ADC_Channel_0, 400, 80);
    vol = adcx * 3300 / 4095;
    temp = (vol + 1) / 10.845f;
    Zx = 10000 * (102.93f - temp) / temp;

    return (u32)Zx;
}

// 测输出阻抗
u32 Get_Out_Zx(void)
{
    u8 cnt;
    float Zx;
    u16 adcx1;
    u16 adcx2;
    float vol1;
    float vol2;
    u8 buff_adc[32];
    
    Relay_Set(2); // 无负载
    delay_ms(50);
    adcx1 = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
    vol1 = adcx1 * 3300 / 4095;

    Relay_Set(3); // 有负载
    delay_ms(50);
    adcx2 = ADC_GetSampleFliter(ADC_Channel_3, 400, 80);
    vol2 = adcx2 * 3300 / 4095;

    Zx = (float)(1597 * (vol1 - vol2) / vol2);
    
    return Zx;
}


// 显示坐标
void LCD_ShowFrame(void)
{
    // 坐标系 x*y  =  280 * 200
    LCD_DrawLine(20, 20, 20, 220, WHITE); // y轴  LCD_H - (Y - 20)
    LCD_DrawLine(20, 220, 300, 220, WHITE); // x轴 X + 20

    /* y轴刻度 */
    LCD_DrawLine(20, 20, 23, 20, WHITE); //
    LCD_ShowString(10, 21, "3", WHITE);
    LCD_DrawLine(20, 87, 23, 87, WHITE); //
    LCD_ShowString(10, 83, "2", WHITE);
    LCD_DrawLine(20, 154, 23, 154, WHITE); //
    LCD_ShowString(10, 150, "1", WHITE);

    /* y轴刻度 */
    LCD_DrawLine(90, 220, 90, 223, WHITE);
    //LCD_ShowString(83, 222, "375K", WHITE);
    LCD_DrawLine(160, 220, 160, 223, WHITE);
    //LCD_ShowString(153, 222, "750K", WHITE);
    LCD_DrawLine(230, 220, 230, 223, WHITE);
    //LCD_ShowString(215, 222, "1125K", WHITE);
    // LCD_ShowString(280, 222, "1.5M", WHITE);

    LCD_ShowString(0, 0, "Amp/V", WHITE);
    LCD_ShowString(302, 200, "F", WHITE);
    LCD_ShowString(10, 220, "0", WHITE);
}


