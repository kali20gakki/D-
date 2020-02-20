/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------�ļ���Ϣ------------------------------------
** @flie    : menu.h
** @auther  : Mrtutu
** @date    : 2019-07-11
** @describe: 
**
**----------------------------------�汾��Ϣ------------------------------------
** �汾����: V0.1
** �汾˵��: ��ʼ�汾
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/
 
#ifndef __MENU_H_
#define __MENU_H_
#include "sys.h"

// ����ѡ����״̬ö��
enum
{
    M_InputImp     = 1,
    M_OutputImp    = 2,
    M_Gain         = 3,
    M_AmpFreq      = 4,
    M_Fault        = 5,
};


// ��������״̬
enum
{
    Task_Menu       = 6,
    Task_InputImp   = 7,
    Task_OutputImp  = 8,
    Task_Gain       = 9,
    Task_AmpFreq    = 10,
    Task_Fault      = 11,
};




//����һ���ṹ��
//��Ž����־λ
typedef struct
{
    u8 Interface_Mark;     //����״̬
    u8 Task_Mark;          //����״̬
} Mark;



extern Mark  Mark_Sign;     //״̬��־λ


#endif

/********************************End of File************************************/