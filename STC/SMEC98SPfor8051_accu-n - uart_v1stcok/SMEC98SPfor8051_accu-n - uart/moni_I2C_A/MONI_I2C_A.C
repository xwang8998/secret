/*********************************************************************************************************************************/
//#include	"STC15Fxxxx.H"
#include	"moni_I2C_A.h"

#include "reg51.h"
#include "intrins.h"
//#include "uart.h"
//#include "gpio.h"

#ifndef uchar 
#define uchar unsigned char 
#endif 
#ifndef uint 
#define uint unsigned int 
#endif 
//void UART_SEND(BYTE dat);
//#define WD7188	0X40
//#define RD7188	0X41 

//sbit	pSDA	=	P1^2;//P3^7;
//sbit	pSCL	=	P1^1;//P3^6;
//sbit	pSDA	=	P2^2;//P2^4;//P3^2;  //7 
//sbit	pSCL	=	P2^1;//P3^3;  //8


bit   bACK;

sbit	IIC_SDA	=	P3^3;//P2^7;
sbit	IIC_SCL	=	P3^2;//P2^6;


//-----------------------------------------------
//========================== 功能子函数===============================
void delay(unsigned int t)
{// 延时函数
while(t){t--;}; //延时循环计数
}
//-------------------------------------------------------------------
void delay_IIC(void)
{//IIC 总线限速延时函数。
//该函数是空函数，延时4 个机器周期。
delay(10);
}
//-------------------------------------------------------------------
void i2c_Init(void)
{//IIC 总线初始化函数
IIC_SDA=1;// 释放IIC 总线的数据线。
IIC_SCL=1;// 释放IIC 总线的时钟线。
}
//-------------------------------------------------------------------
void i2c_start(void)
{//IIC 总线产生起始信号函数
IIC_SDA=1;// 拉高数据线
IIC_SCL=1;// 拉高时钟线
delay_IIC();
IIC_SDA=0;// 在时钟线为高电平时，拉低数据线，产生起始信号。
delay_IIC();
IIC_SCL=0;// 拉低时钟线
}
//-------------------------------------------------------------------
void i2c_stop(void)
{//IIC 总线产生停止信号函数
IIC_SDA=0;// 拉低数据线
delay_IIC();
IIC_SCL=1;// 拉高时钟线。
delay_IIC();
IIC_SDA=1;// 时钟时线为高电平时，拉高数据线，产生停止信号。
delay_IIC();
}
//-------------------------------------------------------------------
bit i2c_Tack(void)
{// 接收应答信号函数
bit ack;// 定义一个位变量，来暂存应答状态。
IIC_SDA=1;// 释放数据总线，准备接收应答信号。
delay_IIC();
IIC_SCL=1;// 拉高时钟线。
delay_IIC();
ack=IIC_SDA;// 读取应答信号的状态。
delay_IIC();
IIC_SCL=0;// 拉低时钟线。
delay_IIC();
return ack;// 返回应答信号的状态， 0 表示应答， 1 表示非应答。
}
//-------------------------------------------------------------------
void i2c_write_byte(unsigned char Data)
{// 向IIC 总线写入一个字节的数据函数
unsigned char i;
for(i=0;i<8;i++)// 有8 位数据
{
IIC_SDA=Data&0x80;// 写最高位的数据
delay_IIC();
IIC_SCL=1; // 拉高时钟线，将数写入到设备中。
delay_IIC();
IIC_SCL=0;// 拉低时钟线，允许改变数据线的状态
delay_IIC();
Data=Data<<1;// 数据左移一位，把次高位放在最高位,为写入次高位做准备
}
}
//-------------------------------------------------------------------
unsigned char i2c_read_byte()
{// 从IIC 总线读取一个字节的数据函数
unsigned char i;
unsigned char Data; //定义一个缓冲寄存器。
for(i=0;i<8;i++)// 有8 位数据
{
IIC_SCL=1;// 拉高时钟线，为读取下一位数据做准备。
delay_IIC();
Data=Data<<1;// 将缓冲字节的数据左移一位，准备读取数据。
delay_IIC();
if(IIC_SDA)// 如果数据线为高平电平。
Data=Data|0x1;// 则给缓冲字节的最低位写1。
IIC_SCL=0;// 拉低时钟线，为读取下一位数据做准备。
delay_IIC();
}


return Data;// 返回读取的一个字节数据。
}
//-------------------------------------------------------------------
void i2c_single_byte_write(unsigned char Daddr,unsigned char Waddr,unsigned char Data)
{// 向任意地址写入一个字节数据函数
i2c_start();// 产生起始信号
i2c_write_byte(Daddr);// 写入设备地址（写）
//FPGA_CTR=0;
i2c_Tack();// 等待设备的应答
//FPGA_CTR=1;
i2c_write_byte(Waddr);// 写入要操作的单元地址。
//FPGA_CTR=0;
i2c_Tack();// 等待设备的应答。
//FPGA_CTR=1;
i2c_write_byte(Data);// 写入数据。
//FPGA_CTR=0;
i2c_Tack();// 等待设备的应答。
//----------------------------------------------------
/*
i2c_write_byte(Data);// 写入数据。
//FPGA_CTR=0;
i2c_Tack();// 等待设备的应答。
i2c_write_byte(Data);// 写入数据。
//FPGA_CTR=0;
i2c_Tack();// 等待设备的应答。
i2c_write_byte(Data);// 写入数据。
//FPGA_CTR=0;
i2c_Tack();// 等待设备的应答。

*/
//------------------------------------------------------
//FPGA_CTR=1;
i2c_stop();// 产生停止符号。

}
//-------------------------------------------------------------------
unsigned char i2c_single_byte_read(unsigned char Daddr,unsigned char Waddr)
{// 从任意地址读取一个字节数据函数
unsigned char Data;//定义一个缓冲寄存器。
i2c_start();// 产生起始信号
i2c_write_byte(Daddr);// 写入设备地址（写）
i2c_Tack();// 等待设备的应答
i2c_write_byte(Waddr);// 写入要操作的单元地址。
i2c_Tack();// 等待设备的应答。
i2c_start();// 产生起始信号
i2c_write_byte(Daddr+1);// 写入设备地址（读） 。
i2c_Tack();// 等待设备的应答。
Data=i2c_read_byte();// 写入数据。
i2c_Tack();
//---------------------------------------------
/*
Data=i2c_read_byte();// 写入数据。
i2c_Tack();
Data=i2c_read_byte();// 写入数据。
i2c_Tack();
Data=i2c_read_byte();// 写入数据。
i2c_Tack();

*/
//-----------------------------------------------

i2c_stop();// 产生停止符号。

//------------------- 返回读取的数据--------------------
return Data;// 返回读取的一个字节数据。
}
