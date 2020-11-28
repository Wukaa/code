#include "reg52.h"

sbit KEY1 = P3 ^ 2;//开始/暂停
sbit KEY2 = P3 ^ 3;//清零
sbit dot = P2 ^ 7;//控制小数点
sbit w1 = P1 ^ 0;
sbit w2 = P1 ^ 1;

typedef unsigned char u8;
typedef unsigned int u16;

u8 ten_s = 0;//10秒位
u8 s = 0, ms = 0;
u16 count_s = 0;
u16 count_ms = 0;
u16 count_ten_s = 0;
u8 flag = 0;//0为s.ms模式，1为ten_s,s模式
u8 flag_stop = 1;

const u8 leddata[] = {//定义const常量的作用，避免数组的值发生改变产生意想不到的结果

                0xC0,  //"0"
                0xF9,  //"1"
                0xA4,  //"2"
                0xB0,  //"3"
                0x99,  //"4"
                0x92,  //"5"
                0x82,  //"6"
                0xF8,  //"7"
                0x80,  //"8"
                0x90,  //"9"
                /**
                0x88,  //"A"
                0x83,  //"B"
                0xC6,  //"C"
                0xA1,  //"D"
                0x86,  //"E"
                0x8E,  //"F"
                0x89,  //"H"
                0xC7,  //"L"
                0xC8,  //"n"
                0xC1,  //"u"
                0x8C,  //"P"
                0xA3,  //"o"
                0xBF,  //"-"
                0xFF,  //熄灭
                0xFF  //自定义
                **/
};

void delay10ms(void)   //误差 0us，
{
    unsigned char a, b, c;
    for (c = 1; c > 0; c--)
        for (b = 38; b > 0; b--)
            for (a = 130; a > 0; a--);
}


void delay50ms(void)   //误差 0us
{
    unsigned char a, b;
    for (b = 173; b > 0; b--)
        for (a = 143; a > 0; a--);
}

void InitTimer0(void)
{
    TMOD = 0x01;
    TH0 = 0x3C;
    TL0 = 0x0B0;
    EA = 1;//总中断开关开启
    ET0 = 1;
    TR0 = 0;//定时器0中断关闭，KEY1按下后开启
    IT0 = 1;//外部中断0开启
    EX0 = 1;//外部中断0下降沿触发
    IT1 = 1;//外部中断1开启
    EX1 = 1;//外部中断1下降沿触发

}

//u8 flag_stop = 1;

void state_control() interrupt 0
{
    //if (!KEY1)
    //{
        delay10ms();//消除抖动
        if (!KEY1)
        {

            TR0 = !TR0;//关闭/开启定时器0
            flag_stop = !flag_stop;

        }
    //}
}

void reset()
{
    TH0 = 0x3C;
    TL0 = 0x0B0;
    count_ten_s = count_s = count_ms = 0;
    ten_s = s = ms = 0;
    flag = 0;//
}

void clear_zero() interrupt 2
{
    //if (!KEY2)
    //{
        delay10ms();
        if (!KEY2)
        {
            if (flag_stop)
            {
                //flag = !flag;
                reset();
            }
        }
   //}
}



/*
u16 count_s = 0;
u16 count_ms = 0;
u16 count_ten_s = 0;
u8 flag = 0;//0为s.ms模式，1为ten_s,s模式
*/


void Timer0Interrupt(void) interrupt 1//50ms定时器中断
{
    TH0 = 0x3C;
    TL0 = 0x0B0;
    if (count_ms == 2)//0.1s=1000ms
    {
        if (ms == 9)
        {
            ms = 0;
        }
        else
        {
            ms++;
        }
        count_ms = 0;
    }

    if (count_s == 20)//1s
    {
        if (s == 9)
        {
            s = 0;
            flag = 1;//开启10秒位
        }
        else
        {
            s++;
        }
        count_s = 0;
    }
    if (count_ten_s == 200)//10s
    {

        if (ten_s == 5)//1min
        {
            ten_s = 0;
        }
        else
        {
            ten_s++;
        }
        count_ten_s = 0;
    }
    count_ms++;
    count_s++;
    count_ten_s++;

}

void display(u8 s_, u8 ms_)
{
    P2 = leddata[s_];
    if (!flag)//是否开启10秒位
    {
        dot = 0;
    }
    w1 = 0;


    w1 = 1;
    dot = 1;
    P2 = leddata[ms_];
    w2 = 0;

    w2 = 1;
}

void select()
{

    switch (flag)
    {
    case 0:display(s, ms); break;
    case 1:display(ten_s, s); break;
        //case 2:stop(); break;
    default:break;
    }
}

void main()
{

    InitTimer0();

    while (1)
    {
        select();

    }

}