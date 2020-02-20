/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : menu.c
** @auther  : Mrtutu
** @date    : 2019-07-11
** @describe: 
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/
#include "menu.h"
#include "ili9341.h"

Mark   Mark_Sign;//状态标志位

/**菜单选择切换界面**/


// 默认菜单
void Menu_MainMenu(void)
{
    LCD_ShowString(75,30,  "1. Input  Imp Test",WHITE);
    LCD_ShowString(75,60,  "2. Output Imp Test",WHITE);
    LCD_ShowString(75,90,  "3. Gain       Test",WHITE);
    LCD_ShowString(75,120, "4. Amp-Freq   Test",WHITE);  
    LCD_ShowString(75,150, "5. Fault      Test",WHITE);
}

// 输入阻抗
void Menu_InputImp(void)
{
    LCD_ShowString(75,30,  "1. Input  Imp Test",RED);
    LCD_ShowString(75,60,  "2. Output Imp Test",WHITE);
    LCD_ShowString(75,90,  "3. Gain       Test",WHITE);
    LCD_ShowString(75,120, "4. Amp-Freq   Test",WHITE);  
    LCD_ShowString(75,150, "5. Fault      Test",WHITE);
}

// 输出阻抗
void Menu_OutputImp(void)
{
    LCD_ShowString(75,30,  "1. Input  Imp Test",WHITE);
    LCD_ShowString(75,60,  "2. Output Imp Test",RED);
    LCD_ShowString(75,90,  "3. Gain       Test",WHITE);
    LCD_ShowString(75,120, "4. Amp-Freq   Test",WHITE);  
    LCD_ShowString(75,150, "5. Fault      Test",WHITE);
}

// 增益
void Menu_Gain(void)
{
    LCD_ShowString(75,30,  "1. Input  Imp Test",WHITE);
    LCD_ShowString(75,60,  "2. Output Imp Test",WHITE);
    LCD_ShowString(75,90,  "3. Gain       Test",RED);
    LCD_ShowString(75,120, "4. Amp-Freq   Test",WHITE);  
    LCD_ShowString(75,150, "5. Fault      Test",WHITE);
}

// 幅频特性
void Menu_AmpFreq(void)
{
    LCD_ShowString(75,30,  "1. Input  Imp Test",WHITE);
    LCD_ShowString(75,60,  "2. Output Imp Test",WHITE);
    LCD_ShowString(75,90,  "3. Gain       Test",WHITE);
    LCD_ShowString(75,120, "4. Amp-Freq   Test",RED);  
    LCD_ShowString(75,150, "5. Fault      Test",WHITE);
}

// 故障检测
void Menu_Fault(void)
{
    LCD_ShowString(75,30,  "1. Input  Imp Test",WHITE);
    LCD_ShowString(75,60,  "2. Output Imp Test",WHITE);
    LCD_ShowString(75,90,  "3. Gain       Test",WHITE);
    LCD_ShowString(75,120, "4. Amp-Freq   Test",WHITE);  
    LCD_ShowString(75,150, "5. Fault      Test",RED);   
}


/********************************End of File************************************/