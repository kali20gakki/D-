/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : menu.h
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
 
#ifndef __MENU_H_
#define __MENU_H_
#include "sys.h"

// 任务选择条状态枚举
enum
{
    M_InputImp     = 1,
    M_OutputImp    = 2,
    M_Gain         = 3,
    M_AmpFreq      = 4,
    M_Fault        = 5,
};


// 任务运行状态
enum
{
    Task_Menu       = 6,
    Task_InputImp   = 7,
    Task_OutputImp  = 8,
    Task_Gain       = 9,
    Task_AmpFreq    = 10,
    Task_Fault      = 11,
};




//创建一个结构体
//存放界面标志位
typedef struct
{
    u8 Interface_Mark;     //界面状态
    u8 Task_Mark;          //任务状态
} Mark;



extern Mark  Mark_Sign;     //状态标志位


#endif

/********************************End of File************************************/