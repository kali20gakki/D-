/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2019, Main, China, Mrtutu.
**                           All Rights Reserved
**
**                                By Mrtutu
**                           Blog: www.mrtutu.cn
**
**----------------------------------文件信息------------------------------------
** @flie    : lcd.c
** @auther  : Mrtutu
** @date    : 2019-06-16
** @describe:
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/

#include "ili9341.h"
#include "bmp.h"
#include "lcdfont.h"

u16 BACK_COLOR;   //背景色

u8 SPI1_ReadWriteByte(u8 TxData);






// SPI1_SCK  --- PB3
// SPI1_MISO --- PB4
// SPI1_MOSI --- PB5
void SPI1_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI1时钟

    //GPIOFB3,4,5初始化设置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; //PB3~5复用功能输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1); //PB3复用为 SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1); //PB4复用为 SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1); //PB5复用为 SPI1

    //这里只针对SPI口初始化
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE); //复位SPI1
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE); //停止复位SPI1

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(SPI1, ENABLE); //使能SPI外设

    SPI1_ReadWriteByte(0xff);//启动传输
}


/*
*@func: SPI1速度设置函数
*@note：SPI速度=fAPB2/分频系数
* 			fAPB2时钟一般为84Mhz
*

*/
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    SPI1->CR1 &= 0XFFC7; //位3-5清零，用来设置波特率
    SPI1->CR1 |= SPI_BaudRatePrescaler;	//设置SPI1速度
    SPI_Cmd(SPI1, ENABLE); //使能SPI1
}


/*
*@func: SPI1 读写一个字节
*@param：TxData:要写入的字节
*@return: 读取到的字节
*

*/
u8 SPI1_ReadWriteByte(u8 TxData)
{

    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {}//等待发送区空

    SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个byte  数据

    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {} //等待接收完一个byte

    return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据

}







/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(u8 dat)
{
    SPI1_ReadWriteByte(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
{
    OLED_DC_Set();//写数据
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(u16 dat)
{
    OLED_DC_Set();//写数据
    LCD_Writ_Bus(dat >> 8);
    LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
    OLED_DC_Clr();//写命令
    LCD_Writ_Bus(dat);
}



/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_WR_REG(0x2a);//列地址设置
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2);
    LCD_WR_REG(0x2b);//行地址设置
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2);
    LCD_WR_REG(0x2c);//储存器写
}



// BLK       --- PB8
// DC        --- PB7
// RES       --- PB6
void LCDx_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    SPI1_Init();

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
    GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);

    OLED_RES_Clr();
    delay_ms(200);
    OLED_RES_Set();
    delay_ms(200);
    OLED_BLK_Set();
    delay_ms(200);

    //************* Start Initial Sequence **********//
    LCD_WR_REG(0xCF);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xC1);
    LCD_WR_DATA8(0X30);
    LCD_WR_REG(0xED);
    LCD_WR_DATA8(0x64);
    LCD_WR_DATA8(0x03);
    LCD_WR_DATA8(0X12);
    LCD_WR_DATA8(0X81);
    LCD_WR_REG(0xE8);
    LCD_WR_DATA8(0x85);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x79);
    LCD_WR_REG(0xCB);
    LCD_WR_DATA8(0x39);
    LCD_WR_DATA8(0x2C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x34);
    LCD_WR_DATA8(0x02);
    LCD_WR_REG(0xF7);
    LCD_WR_DATA8(0x20);
    LCD_WR_REG(0xEA);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_REG(0xC0); //Power control
    LCD_WR_DATA8(0x1D); //VRH[5:0]
    LCD_WR_REG(0xC1); //Power control
    LCD_WR_DATA8(0x12); //SAP[2:0];BT[3:0]
    LCD_WR_REG(0xC5); //VCM control
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x3F);
    LCD_WR_REG(0xC7); //VCM control
    LCD_WR_DATA8(0x92);
    LCD_WR_REG(0x3A); // Memory Access Control
    LCD_WR_DATA8(0x55);
    LCD_WR_REG(0x36); // Memory Access Control
    if(USE_HORIZONTAL == 0)LCD_WR_DATA8(0x08);
    else if(USE_HORIZONTAL == 1)LCD_WR_DATA8(0xC8);
    else if(USE_HORIZONTAL == 2)LCD_WR_DATA8(0x78);
    else LCD_WR_DATA8(0xA8);

    LCD_WR_REG(0xB1);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x12);
    LCD_WR_REG(0xB6); // Display Function Control
    LCD_WR_DATA8(0x0A);
    LCD_WR_DATA8(0xA2);

    LCD_WR_REG(0x44);
    LCD_WR_DATA8(0x02);

    LCD_WR_REG(0xF2); // 3Gamma Function Disable
    LCD_WR_DATA8(0x00);
    LCD_WR_REG(0x26); //Gamma curve selected
    LCD_WR_DATA8(0x01);
    LCD_WR_REG(0xE0); //Set Gamma
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x22);
    LCD_WR_DATA8(0x1C);
    LCD_WR_DATA8(0x1B);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x48);
    LCD_WR_DATA8(0xB8);
    LCD_WR_DATA8(0x34);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x00);
    LCD_WR_REG(0XE1); //Set Gamma
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x23);
    LCD_WR_DATA8(0x24);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x38);
    LCD_WR_DATA8(0x47);
    LCD_WR_DATA8(0x4B);
    LCD_WR_DATA8(0x0A);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x06);
    LCD_WR_DATA8(0x30);
    LCD_WR_DATA8(0x38);
    LCD_WR_DATA8(0x0F);
    LCD_WR_REG(0x11); //Exit Sleep
    delay_ms(120);
    LCD_WR_REG(0x29); //Display on
}




/******************************************************************************
      函数说明：LCD清屏函数
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Clear(u16 Color)
{
    u16 i, j;
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    for(i = 0; i < LCD_W; i++)
    {
        for(j = 0; j < LCD_H; j++)
        {
            LCD_WR_DATA(Color);
        }

    }
}


/******************************************************************************
      函数说明：LCD显示汉字
      入口数据：x,y   起始坐标
                index 汉字的序号
                size  字号
      返回值：  无
******************************************************************************/
void LCD_ShowChinese(u16 x, u16 y, u8 index, u8 size, u16 color)
{
    u8 i, j;
    u8* temp, size1;
    if(size == 32)
    {
        temp = Hzk32;
    }

    LCD_Address_Set(x, y, x + size - 1, y + size - 1); //设置一个汉字的区域
    size1 = size * size / 8; //一个汉字所占的字节
    temp += index * size1; //写入的起始位置
    for(j = 0; j < size1; j++)
    {
        for(i = 0; i < 8; i++)
        {
            if((*temp & (1 << i)) != 0) //从数据的低位开始读
            {
                LCD_WR_DATA(color);//点亮
            }
            else
            {
                LCD_WR_DATA(BACK_COLOR);//不点亮
            }
        }
        temp++;
    }
}


/******************************************************************************
      函数说明：LCD显示汉字
      入口数据：x,y   起始坐标
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
    LCD_Address_Set(x, y, x, y); //设置光标位置
    LCD_WR_DATA(color);
}



/******************************************************************************
      函数说明：LCD画一个大的点
      入口数据：x,y   起始坐标
      返回值：  无
******************************************************************************/
void LCD_DrawPoint_big(u16 x, u16 y, u16 color)
{
    LCD_Fill(x - 1, y - 1, x + 1, y + 1, color);
}


/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
      返回值：  无
******************************************************************************/
void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{
    u16 i, j;
    LCD_Address_Set(xsta, ysta, xend, yend);   //设置光标位置
    for(i = ysta; i <= yend; i++)
    {
        for(j = xsta; j <= xend; j++)LCD_WR_DATA(color); //设置光标位置
    }
}


/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
      返回值：  无
******************************************************************************/
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; //计算坐标增量
    delta_y = y2 - y1;
    uRow = x1; //画线起点坐标
    uCol = y1;
    if(delta_x > 0)incx = 1; //设置单步方向
    else if(delta_x == 0)incx = 0; //垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if(delta_y > 0)incy = 1;
    else if(delta_y == 0)incy = 0; //水平线
    else
    {
        incy = -1;
        delta_y = -delta_x;
    }
    if(delta_x > delta_y)distance = delta_x; //选取基本增量坐标轴
    else distance = delta_y;
    for(t = 0; t < distance + 1; t++)
    {
        LCD_DrawPoint(uRow, uCol, color); //画点
        xerr += delta_x;
        yerr += delta_y;
        if(xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if(yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}



void LCD_DrawLineWidth(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; //计算坐标增量
    delta_y = y2 - y1;
    uRow = x1; //画线起点坐标
    uCol = y1;
    if(delta_x > 0)incx = 1; //设置单步方向
    else if(delta_x == 0)incx = 0; //垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if(delta_y > 0)incy = 1;
    else if(delta_y == 0)incy = 0; //水平线
    else
    {
        incy = -1;
        delta_y = -delta_x;
    }
    if(delta_x > delta_y)distance = delta_x; //选取基本增量坐标轴
    else distance = delta_y;
    for(t = 0; t < distance + 1; t++)
    {
        LCD_DrawPoint_big(uRow, uCol, color); //画点
        xerr += delta_x;
        yerr += delta_y;
        if(xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if(yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}
/*
*********************************************************************************************************
*	函 数 名: OLED_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     ：坐标数组
*			_ucColor ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawPoints(uint16_t* x, uint16_t* y, uint16_t _usSize, uint8_t _ucColor)
{
    uint16_t i;

//    for(i = 0 ; i < _usSize - 1; i++)
//    {
//        LCD_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _ucColor);
//    }

    for(i = 0 ; i < _usSize - 1; i++)
    {
        //LCD_DrawPoint_big(x[i], y[i],_ucColor);
        LCD_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _ucColor);
    }
    delay_ms(4);
    for(i = 0 ; i < _usSize - 1; i++)
    {
        //LCD_DrawPoint_big(x[i], y[i],WHITE);
        LCD_DrawLine(x[i], y[i], x[i + 1], y[i + 1], WHITE);
    }
}

/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}


/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
      返回值：  无
******************************************************************************/
void Draw_Circle(u16 x0, u16 y0, u8 r, u16 color)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    while(a <= b)
    {
        LCD_DrawPoint(x0 - b, y0 - a, color);       //3
        LCD_DrawPoint(x0 + b, y0 - a, color);       //0
        LCD_DrawPoint(x0 - a, y0 + b, color);       //1
        LCD_DrawPoint(x0 - a, y0 - b, color);       //2
        LCD_DrawPoint(x0 + b, y0 + a, color);       //4
        LCD_DrawPoint(x0 + a, y0 - b, color);       //5
        LCD_DrawPoint(x0 + a, y0 + b, color);       //6
        LCD_DrawPoint(x0 - b, y0 + a, color);       //7
        a++;
        if((a * a + b * b) > (r * r)) //判断要画的点是否过远
        {
            b--;
        }
    }
}


/******************************************************************************
      函数说明：显示字符
      入口数据：x,y    起点坐标
                num    要显示的字符
                mode   1叠加方式  0非叠加方式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 mode, u16 color)
{
    u8 temp;
    u8 pos, t;
    u16 x0 = x;
    if(x > LCD_W - 16 || y > LCD_H - 16)return;	 //设置窗口
    num = num - ' '; //得到偏移后的值
    LCD_Address_Set(x, y, x + 8 - 1, y + 16 - 1); //设置光标位置
    if(!mode) //非叠加方式
    {
        for(pos = 0; pos < 16; pos++)
        {
            temp = asc2_1608[(u16)num * 16 + pos];		 //调用1608字体
            for(t = 0; t < 8; t++)
            {
                if(temp & 0x01)LCD_WR_DATA(color);
                else LCD_WR_DATA(BACK_COLOR);
                temp >>= 1;
                x++;
            }
            x = x0;
            y++;
        }
    }
    else //叠加方式
    {
        for(pos = 0; pos < 16; pos++)
        {
            temp = asc2_1608[(u16)num * 16 + pos];		 //调用1608字体
            for(t = 0; t < 8; t++)
            {
                if(temp & 0x01)LCD_DrawPoint(x + t, y + pos, color); //画一个点
                temp >>= 1;
            }
        }
    }
}


/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y    起点坐标
                *p     字符串起始地址
      返回值：  无
******************************************************************************/
void LCD_ShowString(u16 x, u16 y, const u8* p, u16 color)
{
    while(*p != '\0')
    {
        if(x > LCD_W - 16)
        {
            x = 0;
            y += 16;
        }
        if(y > LCD_H - 16)
        {
            y = x = 0;
            LCD_Clear(RED);
        }
        LCD_ShowChar(x, y, *p, 0, color);
        x += 8;
        p++;
    }
}


/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
u32 mypow(u8 m, u8 n)
{
    u32 result = 1;
    while(n--)result *= m;
    return result;
}


/******************************************************************************
      函数说明：显示数字
      入口数据：x,y    起点坐标
                num    要显示的数字
                len    要显示的数字个数
      返回值：  无
******************************************************************************/
void LCD_ShowNum(u16 x, u16 y, u16 num, u8 len, u16 color)
{
    u8 t, temp;
    u8 enshow = 0;
    for(t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len - t - 1)) % 10;
        if(enshow == 0 && t < (len - 1))
        {
            if(temp == 0)
            {
                LCD_ShowChar(x + 8 * t, y, ' ', 0, color);
                continue;
            }
            else enshow = 1;

        }
        LCD_ShowChar(x + 8 * t, y, temp + 48, 0, color);
    }
}


/******************************************************************************
      函数说明：显示小数
      入口数据：x,y    起点坐标
                num    要显示的小数
                len    要显示的数字个数
      返回值：  无
******************************************************************************/
void LCD_ShowNum1(u16 x, u16 y, float num, u8 len, u16 color)
{
    u8 t, temp;
    u8 enshow = 0;
    u16 num1;
    num1 = num * 100;
    for(t = 0; t < len; t++)
    {
        temp = (num1 / mypow(10, len - t - 1)) % 10;
        if(t == (len - 2))
        {
            LCD_ShowChar(x + 8 * (len - 2), y, '.', 0, color);
            t++;
            len += 1;
        }
        LCD_ShowChar(x + 8 * t, y, temp + 48, 0, color);
    }
}


/******************************************************************************
      函数说明：显示40x40图片
      入口数据：x,y    起点坐标
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(u16 x1, u16 y1, u16 x2, u16 y2)
{
    int i;
    LCD_Address_Set(x1, y1, x2, y2);
    for(i = 0; i < 1600; i++)
    {
        LCD_WR_DATA8(image[i * 2 + 1]);
        LCD_WR_DATA8(image[i * 2]);
    }
}









void LCD_Show_R1_Open(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 1, 32, RED);
    LCD_ShowChinese(154, 90, 6, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}

void LCD_Show_R2_Open(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 2, 32, RED);
    LCD_ShowChinese(154, 90, 6, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}

void LCD_Show_R3_Open(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 3, 32, RED);
    LCD_ShowChinese(154, 90, 6, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}


void LCD_Show_R4_Open(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 4, 32, RED);
    LCD_ShowChinese(154, 90, 6, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}


void LCD_Show_R1_Short(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 1, 32, RED);
    LCD_ShowChinese(154, 90, 8, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}

void LCD_Show_R2_Short(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 2, 32, RED);
    LCD_ShowChinese(154, 90, 8, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}

void LCD_Show_R3_Short(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 3, 32, RED);
    LCD_ShowChinese(154, 90, 8, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}

void LCD_Show_R4_Short(void)
{
    LCD_ShowChinese(90, 90, 0, 32, RED);
    LCD_ShowChinese(122, 90, 4, 32, RED);
    LCD_ShowChinese(154, 90, 8, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}


void LCD_Show_C1_Open(void)
{
    LCD_ShowChinese(90, 90, 5, 32, RED);
    LCD_ShowChinese(122, 90, 1, 32, RED);
    LCD_ShowChinese(154, 90, 6, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}


void LCD_Show_C2_Open(void)
{
    LCD_ShowChinese(90, 90, 5, 32, RED);
    LCD_ShowChinese(122, 90, 2, 32, RED);
    LCD_ShowChinese(154, 90, 6, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}


void LCD_Show_C3_Open(void)
{
    LCD_ShowChinese(90, 90, 5, 32, RED);
    LCD_ShowChinese(122, 90, 3, 32, RED);
    LCD_ShowChinese(154, 90, 6, 32, RED);
    LCD_ShowChinese(184, 90, 7, 32, RED);
}



void LCD_Show_C1_x2(void)
{
    LCD_ShowChinese(90, 90, 5, 32, RED);
    LCD_ShowChinese(122, 90, 1, 32, RED);
    LCD_ShowChinese(154, 90, 13, 32, RED);
    LCD_ShowChinese(184, 90, 14, 32, RED);
}



void LCD_Show_C2_x2(void)
{
    LCD_ShowChinese(90, 90, 5, 32, RED);
    LCD_ShowChinese(122, 90, 2, 32, RED);
    LCD_ShowChinese(154, 90, 13, 32, RED);
    LCD_ShowChinese(184, 90, 14, 32, RED);
}



void LCD_Show_C3_x2(void)
{
    LCD_ShowChinese(90, 90, 5, 32, RED);
    LCD_ShowChinese(122, 90, 3, 32, RED);
    LCD_ShowChinese(154, 90, 13, 32, RED);
    LCD_ShowChinese(184, 90, 14, 32, RED);
}



void LCD_Show_Normal(void)
{
    LCD_ShowChinese(90, 90, 9, 32, RED);
    LCD_ShowChinese(122, 90, 10, 32, RED);
    LCD_ShowChinese(154, 90, 11, 32, RED);
    LCD_ShowChinese(184, 90, 12, 32, RED);
}



/********************************End of File************************************/