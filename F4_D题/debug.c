//        if(Uo > 11.0) // 具体电压待测
//        {
//            if(fabs(Uo - Vcc) < 0.2) // 输出约等于 电源电压
//            {
//                // Rc 短路
//                LCD_ShowString(75, 40, "Rc  Short Circuit", RED);

//            }
//            else if(fabs(Uo - 11.2) < 0.2) // 输出约等于 11.2 约为 (12 - 0.7)
//            {
//                // Rb1 短路  ???  11.2v
//                LCD_ShowString(75, 40, "Rb1 Short Circuit", RED);
//            }

//            /*截止*****************************/
//            else
//            {
//                // 测输入阻抗
//                IN_Zx = Get_IN_Zx();

//                if(abs(IN_Zx - 25) < 100)// In == 25
//                {
//                    // Rb2 短路
//                    LCD_ShowString(75, 40, "Rb2 Short Circuit", RED);
//                }
//                else if(IN_Zx < 13000)
//                {
//                    // Re开路  输入阻抗变大
//                    LCD_ShowString(75, 40, "Re  Open  Circuit", RED);
//                }
//                else if(IN_Zx >= 13000)// Uo(输入阻抗明显大)  ???
//                {
//                    // Rb1 开
//                    LCD_ShowString(75, 40, "Rb1 Open  Circuit", RED);
//                }

//            }
//        }
//        /*饱和*************************************/
//        else if(Uo <= 5) // 具体电压待测  ???
//        {
//            if(fabs(Uo - 4.2) < 0.2) // In 大  4.几V   输入阻抗200欧
//            {
//                // 测输入阻抗
//                IN_Zx = Get_IN_Zx();
//                if(abs(IN_Zx - 200) < 100)//阻抗200欧左右
//                {
//                    // Rb2 开路  ???
//                    LCD_ShowString(75, 40, "Rb2  Open Circuit", RED);
//                }
//            }
//            else if(Uo < 0.4)
//            {
//                // Rc 开
//                LCD_ShowString(75, 40, "Rc  Open  Circuit", RED);
//            }
//        }
//        /*正常放大*************************************/
//        else
//        {
//            // 测输入阻抗
//            IN_Zx = Get_IN_Zx();
//            if(IN_Zx > 8000) //if In 很大
//            {
//                //测放大倍数
//                Gain = Get_Gain();
//                if(Gain < 15) //放大倍数小  6
//                {
//                    // C2 开
//                    LCD_ShowString(75, 40, "C2  Open  Circuit", RED);
//                }
//                else
//                {
//                    LCD_ShowString(75, 40, "Normal1           ", RED);
//                }
//            }
//            else if(100 <IN_Zx < 1200) // In小于 1K
//            {
//                // Re 短路
//                LCD_ShowString(75, 40, "Re  Short  Circuit", RED);
//            }
//            else if((IN_Zx - Normal_IN_Zx) < 0 && abs(IN_Zx - Normal_IN_Zx) > 200) //  In 变小
//            {
//                // 测放大倍数
//                Gain = Get_Gain();
//                if((Gain - Normal_Gain) > 3) // 放大倍数增大
//                {
//                    // C2 变大
//                    LCD_ShowString(75, 40, "C2     Twice", RED);
//                }
//                else
//                {
//                    // C1 变大
//                    LCD_ShowString(75, 40, "C1     Twice", RED);
//                }
//            }
//            else
//            {
//                // 输入交流
//                Relay_Set(7);
//                printf("1000\r\n");
//                delay_ms(200);
//                // 测Uo
//                //Uo = Get_DC_Uo();
//                if(Uo_is_HalfWave() == 0) // Uo无波形
//                {
//                    // C1 开
//                    LCD_ShowString(75, 40, "C1  Open  Circuit", RED);
//                }
//                else
//                {
//                    Gain = Get_Gain();
//                    printf("300000\r\n"); // 打300K
//                    delay_ms(20);
//                    Gain_300K = Get_Gain();

//                    if(Gain_300K - Gain > 30) //   ????   打300K 比较 有无 的放大倍数  变大
//                    {
//                        // C3 开
//                        LCD_ShowString(75, 40, "C3  Open  Circuit", RED);
//                    }
//                    else if(Gain_300K - Gain < -10)//截止变小         打300K 比较 有无 的放大倍数  变小
//                    {
//                        // C3 变大
//                        LCD_ShowString(75, 40, "C3        Twice ", RED);
//                    }
//                    else
//                    {
//                        // 正常
//                        LCD_ShowString(75, 40, "Normal2         ", RED);
//                    }
//                }

//            }
//        }