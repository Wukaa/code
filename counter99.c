#include "reg52.h"
/**************************************************************
	计时器功能：
	1、KEY1开始/暂停和设置初始值(按一次加一)
	2、KEY2清零/ 控制	进入开始模式and设置计数初值模式
	3、设置初始数字
	4、计数范围0-99s
	初始状态为 KEY1设置初值	KEY2模式切换
**************************************************************/

sbit KEY1 = P3 ^ 2;//开始/暂停
sbit KEY2 = P3 ^ 3;//清零
sbit dot = P0 ^ 7;//控制小数点
sbit w1 = P1 ^ 2;
sbit w2 = P1 ^ 3;

typedef unsigned char u8;
typedef unsigned int u16;

u8 ten_s = 0;//10秒位
u8 s = 0;
u16 count_s = 0;
u8 flag_stop = 1;//1为暂停，0为开始,多余？？？
u8 mode_display = 0;//用于判断模式二是否开启
u8 mode = 1;//1为暂停，0为开始,控制KEY1是(开始/暂停)还是设置初值

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

void InitTimer0(void)
{
	EA = 1;//总中断开关开启

	TMOD = 0x01;//定时器0	工作方式1	16位计数
	TH0 = 0x3C;//0x3cb0=（65535-50000）（十进制）， 15536/256=60=0x3c，15536%256=176=0xb0
	TL0 = 0x0B0;//定时器0初值	50000
	ET0 = 1;//定时器0中断允许开启
	TR0 = 0;//定时器0中断关闭，KEY1按下后开启

	IT0 = 1;//外部中断0开启
	EX0 = 1;//外部中断0下降沿触发

	IT1 = 1;//外部中断1开启
	EX1 = 1;//外部中断1下降沿触发
}

void reset()
{
	TH0 = 0x3C;
	TL0 = 0x0B0;
	count_s =  0;
	ten_s = s =  0;
	//flag = 0;//
}

//u8 flag_stop = 0;//1为暂停，0为开始
//u8 mode_display = 0;//用于判断模式二是否开启
//u8 mode = 1;//1为暂停，0为开始,控制KEY1是(开始/暂停)还是设置初值

void state_control_count_set() interrupt 0//开始/暂停/设置初始值
{

	delay10ms();
	if (!KEY1)
	{
		if (mode)//模式1,按一下数码管数字增加1
		{
			if (s == 9)
			{
				s = 0;
				if (ten_s == 9)
				{
					ten_s = 0;
				}
				else
				{
					ten_s++;//向ten_s进位
				}
			}
			else
			{
				s++;
			}
		}
		else//模式2，开始/暂停功能
		{
			TR0 = !TR0;
			flag_stop = !flag_stop;
		}
	}

}

//u8 flag_stop = 0;//0为暂停，1为开始
//u8 mode_display = 0;//用于判断模式二是否开启
//u8 mode = 1;//1为暂停，0为开始,控制KEY1是(开始/暂停)还是设置初值
void clear_zero() interrupt 2//清零
{
	delay10ms();
	if (!KEY2)
	{
		//TR0 = !TR0;
		if (flag_stop)//flag_stop == 1暂停状态下
		{
			if(!mode)
			{
				reset();
			}
			mode_display = !mode_display;
			mode = !mode;//模式切换
		}
	}
}

/*
u8 ten_s = 0;//10秒位
u8 s = 0;
*/
void timer0() interrupt 1//50毫秒计时
{
	TH0 = 0x3C;
	TL0 = 0x0B0;
	if (count_s == 20)//1s
	{
		if (ten_s == 0 && s == 0)
		{
			TR0 = 0;//有问题？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
			TH0 = 0x3C;
			TL0 = 0x0B0;
			mode = !mode;//变为设置初值模式
			mode_display = !mode_display;//计时结束变为模式
			flag_stop = !flag_stop;//数值变为0后设置初值,KEY1按下不能开始的bug(已解决)
		}
		else
		{
			if (s == 0)
			{
				s = 9;
				if (ten_s != 0)
				{
					ten_s--;//向ten_s借位
				}
			}
			else
			{
				s--;
			}
		}
		count_s = 0;
	}
	if (!mode)
	{
		count_s++;
	}
}

void display(u8 tens_, u8 s_)
{//小数点不亮
	P0 = leddata[tens_];
	w1 = 0;
	w1 = 1;
	P0 = leddata[s_];
	dot = mode_display;
	w2 = 0;
	w2 = 1;
}

void main()
{
	InitTimer0();
	while (1)
	{
		display(ten_s, s);
	}
}