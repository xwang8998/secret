//stcioset_demo1.c V2018.03 使用KEIL C51 u2及以上版本编译通过

#include "STC8G.H" //请根据单片机型号选择头文件
//#include <STC15F2K60S2.H>

#include "string.h"
#include <stdlib.h>
#include <SMEC98SP.h>
#include <iic_smec98sp.h>
//#include "my.h"
#include <moni_I2C_A.h>


#define FPGA_ADDR	0xA0
#define HIGH	1
#define LOW		0
#define IIC_NOACK 1
#define IIC_ACK 0
#define IIC_DELAYTIME 10			//IIC通讯速率大约100K
#define IIC_ADDR	0x00			//对应SMEC98SP中地址
#define ACKCHECKTIME 600
#define BAUD  0xFEC8  
typedef bit BOOL;
typedef unsigned char BYTE;
typedef unsigned int WORD;

BYTE TBUF,RBUF;
BYTE TDAT,RDAT;
BYTE TCNT,RCNT;
BYTE TBIT,RBIT;
BOOL TING,RING;
BOOL TEND,REND;

#define STACK_MAX  16
unsigned char gfail;
//unsigned char gi, gj,gk,gm,gn,gret,gfail; //changed linsd //xdata
//unsigned char gi, gj,gk,gm,gn,gret,gfail,; //changed linsd //xdata
 //unsigned char code InternalKey[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};//内部认证密钥,必须和SMEC98SP一致
// unsigned char buf[20];
unsigned short gx_checksum,xuint,gx_checksum_get;


/******************************************************************************************
/*延时us函数22.1184M  1T
/******************************************************************************************/

                 //38400bps @ 11.0592MHz
sbit    IO_INOUT    =P5^4;			//定义IIC inout管脚，
sbit    IIC_SCL     =P3^2;			//定义IIC SCL管脚
sbit    IIC_SDA     =P3^3;			//定义IIC SDA管脚，
sbit    EN     =P5^5;			//定义IIC en管脚，
sbit 		RXB = P3^0;                        //define UART TX/RX port
sbit 		TXB = P3^1;




void IIC_Delayus(unsigned int n);
void UART_SEND(BYTE dat);
extern unsigned char i2c_single_byte_read(unsigned char xdevaddr,unsigned char RdAddr);
extern void  i2c_single_byte_write(unsigned char xdevaddr,unsigned char xbyte_addr,unsigned char xbyte_data);

void StartUART( void );
void Starttimer0(void);
void delay_nms(unsigned int i);
void print_string(char *str);
void PrintHex(unsigned char *str,unsigned int len);
void UART_INIT();

//BYTE t, r;
BYTE buf[20];
//-----------------------------------------
//Timer interrupt routine for UART

void tm0() interrupt 1 using 1
{
	
    if (RING)
    {
        if (--RCNT == 0)
        {
            RCNT = 3;                   //reset send baudrate counter
            if (--RBIT == 0)
            {
                RBUF = RDAT;            //save the data to RBUF
                RING = 0;               //stop receive
                REND = 1;               //set receive completed flag
            }
            else
            {
                RDAT >>= 1;
                if (RXB) RDAT |= 0x80;  //shift RX data to RX buffer
            }
        }
    }
    else if (!RXB)
    {
        RING = 1;                       //set start receive flag
        RCNT = 4;                       //initial receive baudrate counter
        RBIT = 9;                       //initial receive bit number (8 data bits + 1 stop bit)
    }

    if (--TCNT == 0)
    {
        TCNT = 3;                       //reset send baudrate counter
        if (TING)                       //judge whether sending
        {
            if (TBIT == 0)
            {
                TXB = 0;                //send start bit
                TDAT = TBUF;            //load data from TBUF to TDAT
                TBIT = 9;               //initial send bit number (8 data bits + 1 stop bit)
            }
            else
            {
                TDAT >>= 1;             //shift data to CY
                if (--TBIT == 0)
                {
                    TXB = 1;
                    TING = 0;           //stop send
                    TEND = 1;           //set send completed flag
                }
                else
                {
                    TXB = CY;           //write CY to TX port
                }
            }
        }
    }
}

//-----------------------------------------
//initial UART module variable

void UART_INIT()
{
    TING = 0;
    RING = 0;
    TEND = 1;
    REND = 0;
    TCNT = 0;
    RCNT = 0;
}
void UART_SEND(BYTE dat)
{
    while (!TEND);
    TEND = 0;
    TBUF = dat;
    TING = 1;
}
/*
void UART_HEX(unsigned char *buf, unsigned char len)
{
	unsigned int i;
    for(i=0;i<len;i++)
		{
			UART_SEND(buf[i]);
		}
}
*/
void IIC_Delayus(unsigned int n)
{
	//unsigned char i;
	for(;n>0;n--)
	{
 		;//i = 1;
	}	
}
void    IIC_SetSDA(unsigned char level)
{
	if(level == HIGH)
	{
		IIC_SDA = 1;
	}else
	{
		IIC_SDA = 0;
	}
}
void    IIC_SetSCL(unsigned char level)
{
	if(level == HIGH)
	{
		IIC_SCL = 1;
	}else
	{
		IIC_SCL = 0;
	}
}
void SetSDAto_InputMode(void)
{
	//根据芯片手册配置
	IIC_SDA = 1;			//设为输入
	IO_INOUT = 0;
	
}
void SetSDAto_OutputMode(void)
{
	//根据芯片手册配置
	//本单片机不用设置可直接输出
	IO_INOUT = 1;
	
}

void   IIC_Start(void)
{
	SetSDAto_OutputMode();			//SDA设为输出模式
	IIC_SetSDA(HIGH);
	IIC_SetSCL(HIGH);
	IIC_Delayus(IIC_DELAYTIME);			
	IIC_SetSDA(LOW);
	IIC_Delayus(IIC_DELAYTIME);
	IIC_SetSCL(LOW);
}

void   IIC_Stop(void)
{	
	SetSDAto_OutputMode();			//SDA设为输出模式
	IIC_SetSDA(LOW);
	IIC_SetSCL(HIGH);
	IIC_Delayus(IIC_DELAYTIME);
	IIC_SetSDA(HIGH);
	IIC_Delayus(IIC_DELAYTIME);
}
unsigned char IIC_GetSdaStus(void)
{
	unsigned char tmp;

	if(IIC_SDA == 1)	
		tmp=1;
	else 
		tmp=0;
	return tmp;
}


unsigned char  IIC_Send(unsigned char IIC_data)
{
	unsigned char Ack ;
	unsigned int AckLoop;
	unsigned char xtmp;
	unsigned char i;
	SetSDAto_OutputMode();			//SDA设为输出模式
    //xtmp = IIC_data;//50;
	//while(1)
	//{IIC_SDA = 1;
     //IIC_Delayus(8);
	 //IIC_SDA = 0;
     //IIC_Delayus(8);
	 //IIC_SDA = 0;
     //IIC_Delayus(8);
	//}
    //while(1){
	xtmp = IIC_data;//50;
	for(i=0;i<8;i++)
	{
		IIC_SetSCL(LOW);
		IIC_Delayus(IIC_DELAYTIME);
		if((xtmp&0x80)==0)
		   IIC_SetSDA(0);
		else
		   IIC_SetSDA(1);
		IIC_Delayus(IIC_DELAYTIME);
		IIC_SetSCL(HIGH);
		IIC_Delayus(IIC_DELAYTIME);
		IIC_Delayus(IIC_DELAYTIME);
		xtmp<<= 1;
	}
	//IIC_Delayus(1000);
   //}
	
	IIC_SetSCL(LOW);
	IIC_Delayus(IIC_DELAYTIME);
	IIC_Delayus(IIC_DELAYTIME);
    SetSDAto_InputMode();//SDA设为输入模式
	IIC_SetSCL(HIGH);
	IIC_Delayus(IIC_DELAYTIME);
	IIC_Delayus(IIC_DELAYTIME);
	Ack = IIC_NOACK;
				
	for(AckLoop=0;AckLoop<ACKCHECKTIME;AckLoop++) //260us
	{
		if(!IIC_GetSdaStus())
		{
			Ack = IIC_ACK;
			break;
		}
		IIC_Delayus(IIC_DELAYTIME);
	}
	IIC_SetSCL(LOW);
	
	
	return Ack;			//return success=0 / failure=1
}

unsigned char IIC_Read(unsigned char bACK)
{
	unsigned char Data;
	unsigned char i;
	Data = 0;
	SetSDAto_InputMode();			//SDA设为输入模式
	for(i=0;i<8;i++)
	{
		Data <<= 1;
		IIC_SetSCL(LOW);
		IIC_Delayus(IIC_DELAYTIME);
		IIC_Delayus(IIC_DELAYTIME);
		IIC_Delayus(IIC_DELAYTIME);
		IIC_SetSCL(HIGH);
		IIC_Delayus(IIC_DELAYTIME);
		IIC_Delayus(IIC_DELAYTIME);
		if(IIC_GetSdaStus())
			Data |= 1;
		else
			Data &= 0xfe;
	}
	IIC_SetSCL(LOW);
	
	IIC_Delayus(IIC_DELAYTIME);   
  	IIC_SetSDA(bACK);
	IIC_Delayus(IIC_DELAYTIME);
    SetSDAto_OutputMode();			//SDA设为输出模式	
	IIC_SetSCL(HIGH);
	IIC_Delayus(IIC_DELAYTIME);
	IIC_Delayus(IIC_DELAYTIME);
	IIC_SetSCL(LOW);
	return Data;
}


bit IIC_ReadWithAddr(unsigned char addr, unsigned char *buf, unsigned char len)
{
  	unsigned char i;
	
	
	i = 10;
    while (i) 
	{
		
		IIC_Start();
        if (IIC_Send(addr) == IIC_ACK) break;
        if (--i == 0) return IIC_NOACK;
    }
	for(i = 0; i < len -1 ; i++)
	{
		buf[i] = IIC_Read(IIC_ACK);
	}
	buf[i] = IIC_Read(IIC_NOACK);
	IIC_Stop();

	return IIC_ACK;
}

bit IIC_WriteWithAddr(unsigned char addr, unsigned char *buf, unsigned char len)
{
  	unsigned char i,xaddr,get_ret;
	i = 10;
	xaddr = addr;//0x50;
	
    while (i>0) //for(i = 0; i < 10; i++)//
	{
		//IIC_Send(xaddr);
		
		IIC_Start();
		get_ret = IIC_Send(xaddr);
		if(get_ret==IIC_ACK){break;}
        if (IIC_Send(xaddr) == IIC_ACK) break;
		i--;
        if (i >= 10){
			return IIC_NOACK;}
    }
	for(i = 0; i < len; i++)
	{
		
		get_ret = IIC_Send(buf[i]);
		if(get_ret!=0)
			return IIC_NOACK;
	}
    
	IIC_Delayus(IIC_DELAYTIME);
	IIC_Delayus(IIC_DELAYTIME);
	
	IIC_Stop();
    
	
	
	return IIC_ACK;
}
//----------------------------------------------------------------------------------------
//22.1184M  1T   
void delay_nms(unsigned int i)
{
	unsigned int  j;	
	
	for(;i>0;i--)
	{
	   	j = 1580;
		while(--j);	
			
	}
}
void sd_delay(unsigned char x)
{
	unsigned char  i;
	i = x;
	while(--i);
}


//-----------------------------------------
void  accu_SMEC_GetRandom(void)
{
	unsigned char k,ret,i;
	buf[0] = 0x84;buf[1] = 0x00;buf[2] = 0x00;buf[3] = 14;k=14;
	
	
	ret = IIC_WriteWithAddr(IIC_ADDR & 0xFE, buf, 4);
    /*
	if(!ret)
	{
	UART_SEND(buf[0]);
	UART_SEND(buf[1]);
	UART_SEND(buf[2]);
	UART_SEND(buf[3]);
	
    }
	
	*/
	
	ret = IIC_ReadWithAddr(IIC_ADDR | 0x1, buf, (k+2)); 
	//if(ret || buf[0] != 0x90 || buf[1] != 0x00)
	/*
	if(!ret)
	{
	for(i=0;i<16;i++)
	{
	UART_SEND(buf[i]);
	}
	
    }
	*/
    //}
	
			
}

void checksum_ext(void)
{
	unsigned char  i,k,m,n,j;

	gx_checksum=0;
	for(i=0;i<12;i++)
	   {gx_checksum+=buf[i];}
  gx_checksum *= 7;
  //-----------------------
  gx_checksum=~gx_checksum;
  //----------------------
  k=gx_checksum >> 8;
	m=k >> 3; n = k<<(8-3); j= m | n;
    xuint=j; xuint<<=8;
	gx_checksum &= 0x00ff; gx_checksum|=xuint;
  k=gx_checksum; 
	m=k >> 2; n = k<<(8-2); j= m | n;
	gx_checksum = gx_checksum & 0xff00; xuint=j;gx_checksum|=xuint;
  //---------------------
}
//UART_SEND(0xff);UART_SEND(0xff);UART_SEND(3);UART_SEND(0xff);UART_SEND(0xff);
void fake_check(void)
{
	unsigned char  i,j,ret,k;
	accu_SMEC_GetRandom();
	//---- 生成随机数存在buf[0..11]--------
	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(2);UART_SEND(0xff);UART_SEND(0xff);
  for(i=0;i<12;i++)		
			{k=buf[2+i];sd_delay(k);buf[i]=TL0%8;UART_SEND(buf[i]);}
  k=buf[3];sd_delay(k);buf[12]=TL0%8;
  k=buf[7];sd_delay(k);buf[13]=TL0%8;
	//----- 打包发送 -------------------
  j=13;
  for(i=0;i<14;i++)
			{buf[j+4] = buf[j]; j--;}
  buf[0]=0xa5; buf[1]=0; buf[2]=0;buf[3]=14+4;
  ret = IIC_WriteWithAddr(IIC_ADDR & 0xFE, buf, (12+2+4));	
	
	//if(ret == 1) {while(1){;}}
	if(!ret)
	{
	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(3);UART_SEND(0xff);UART_SEND(0xff);
	for(i=0;i<16;i++)
	{
	
	UART_SEND(buf[i]);
	}
	}
  //----- 延时后读取反馈 -------------
	delay_nms(500);
	ret = IIC_ReadWithAddr(IIC_ADDR | 0x1, buf, (14 + 4));
	//if(ret == 1) {while(1){;}}
	if(!ret)
	{
	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(4);UART_SEND(0xff);UART_SEND(0xff);
	for(i=0;i<16;i++)
	{
	UART_SEND(buf[i]);
	}
	}
}

//-------------------------------------------------------------------------------------------
unsigned char SMEC_GetUid(unsigned char * pUID)
{
	unsigned char bBuf[16] = {0};
	unsigned char ret;
	unsigned char  len;

	bBuf[0] = 0x83;		        //读UID命令字，可更改，与加密芯片中程序保持一致
	bBuf[1] = 0x00;
	bBuf[2] = 0x00;
	bBuf[3] = len = 0x0c;		//需要获取的UID长度
	UART_SEND(bBuf[0]);
	UART_SEND(bBuf[1]);
	UART_SEND(bBuf[2]);
	UART_SEND(bBuf[3]);
	ret = IIC_WriteWithAddr(IIC_ADDR & 0xFE, bBuf, 4);
	if(ret)                     //出错
		return 1;

	//说明：若正确获取UID, 加密芯片返回数据90 00 + UID1 ~ UID12．　前2字节为状态码, 0x9000 表示执行正确, 其他表示错误
	ret = IIC_ReadWithAddr(IIC_ADDR | 0x1, bBuf, (len + 2));
	
	if(ret || bBuf[0] != 0x90 || bBuf[1] != 0x00)         //通信错误或状态码不为0x9000
		return 1;		

	memcpy(pUID, bBuf + 2, len);
	return 0;		            //读取成功
}
//---------------------------------------------------------
//函数名: 验证加密芯片SMEC98SP的PIN码
//参数说明：
//			pbPin - PIN码
//          bPinLen - PIN长度
//返回值说明：
//			0 - 成功
//			1 - 失败
//---------------------------------------------------------
unsigned char SMEC_CheckPin(unsigned char *pbPin, unsigned char bPinLen)
{
	unsigned char bBuf[16] = {0};
	unsigned char  ret;

	bBuf[0] = 0x70;		        //可更改，与加密芯片中程序保持一致
	bBuf[1] = 0x00;
	bBuf[2] = 0x00;
	bBuf[3] = bPinLen;	    //需要获取的随机数长度
	
	ret = IIC_WriteWithAddr(IIC_ADDR & 0xFE, bBuf, bPinLen + 4);
	
	
	//PrintHex(bBuf, bPinLen + 4);
	if(ret)                     //出错
		return 1;

	//若正确验证PIN 加密芯片返回 90 00
	ret = IIC_ReadWithAddr(IIC_ADDR | 0x1, bBuf, 2); 
	
	//PrintHex(bBuf, 2);
	if(ret || bBuf[0] != 0x90 || bBuf[1] != 0x00)         //通信错误或状态码不为0x9000
		return 1;		

	return 0;		            //验证PIN成功
}

//---------------------------------------------------------

void SMEC_Test(void)
{
	/*各种密钥,不会在I2C线路上传输,可以使用同一组.应该将密钥分散存储,防止主控芯片被破解后,被攻击者在二进制码中找到密钥 */
//	unsigned char InternalKey[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};//内部认证密钥,必须和SMEC98SP一致
//	unsigned char ExternalKey[16] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};//外部认证密钥,必须和SMEC98SP一致
//	unsigned char SHA1_Key[16] = {0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F};   //哈希算法认证密钥,必须和SMEC98SP一致
//	unsigned char MKey[16] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F};   //主控密钥,用于产生过程密钥,必须和SMEC98SP一致

	unsigned char Pin[8] = {0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc};			//Pin认证密钥,必须和SMEC98SP一致
		
	//unsigned char bSmec98spUid[12] = {0};		//存放SMEC98SP的UID
	//unsigned short RandomSeek = 0;				//随机数种子
	//unsigned char bRandom[8] = {0};				//存放随机数
	//unsigned char bSessionKey[8] = {0};			//存放过程密钥,过程密钥为临时产生的密钥
	//unsigned char bDataBuf[64] = {0};
	unsigned char ret;

	
	 
	
	//获取SMEC98SP的UID
	//ret = SMEC_GetUid(bSmec98spUid);
	ret = SMEC_CheckPin(Pin, (unsigned char)sizeof(Pin));


}


//////////////////////////////////////////////////////////////////////
void main(void)
{
	unsigned char  ret,i,k,j,flag;
	P3M0 = 0x00;
	P3M1 = 0x00;
	P5M1= 0x00;
	P5M0 =0x00;
	
//	P_SW2 =0x80;
//	CKSEL =0x00;
//	P_SW2 =0x00;
/*
	while(1){
	IO_INOUT = ~IO_INOUT;
	EN = ~EN;
	//IIC_SCL=~IIC_SCL;
	//IIC_SDA = ~IIC_SDA;
	IIC_Send(0X01);
	//delay_nms(10);
	}
	*/

	TMOD = 0xf0;                        //timer0 in 16-bit auto reload mode
    AUXR = 0x80;                        //timer0 working at 1T mode
    TL0 = 0x30;//0x8f+0x05;//BAUD;
    TH0 = 0xff;//0xfd;//BAUD>>8;                      //initial timer0 and set reload value
	TF0 =0;
    TR0 = 1;                            //tiemr0 start running
    ET0 = 1;                            //enable timer0 interrupt
    PT0 = 1;                            //improve timer0 interrupt priority
    EA = 1;                             //open global interrupt switch

	
    UART_INIT();
	EN = 1;	
  
	gfail=0;
	//gstacki = 0;
	
while(1)
	{

	
	if(EN == 1)
	{
	IO_INOUT = 0;
	delay_nms(1);
	UART_SEND(0xaa);
	delay_nms(1);
	UART_SEND(0x01);
    delay_nms(1);	
	IO_INOUT = 1;
	
	delay_nms(1);
	IO_INOUT = 0;	
	IO_INOUT = 1;
	
	delay_nms(1);
//-------------------------------------------------------------------------
//delay_nms(12000);			//等待加密芯片上电复位完成
	//---------- 伪校验 ----------------------------------------
	//SMEC_Test();
	//fake_check();
	delay_nms(300);
	//UART_SEND(0xff);UART_SEND(0xff);UART_SEND(5);UART_SEND(0xff);UART_SEND(0xff);

	delay_nms(300);
	fake_check();
	delay_nms(300);
	fake_check();
	delay_nms(300);
	fake_check();
	delay_nms(300);
	fake_check();
	delay_nms(300);	 
	
    
	//----- get random -------

	accu_SMEC_GetRandom();
	//if(ret == 1) {while(1){;}}
	//--------------------------------------
	//---- 生成随机数存在buf[0..11]--------
  for(i=0;i<12;i++)		
			{k=buf[2+i];sd_delay(k);buf[i]=TL0%8;}
	//------ 用求和加密 ------------------
	flag++;
	if(flag>100){flag = 0;}
	if(flag%2 == 0)
	{
	checksum_ext();
	}
  //----- 打包发送 3 7-------------------
  j=11;
  for(i=0;i<12;i++)
			{buf[j+4] = buf[j]; j--;}
  buf[0]=0xa5; buf[1]=0; buf[2]=0;buf[3]=12+2+4;
	buf[12+4]=buf[3+4]; buf[3+4]=gx_checksum>>8; 
	buf[12+4+1]=buf[7+4]; buf[7+4]=gx_checksum;
	ret = IIC_WriteWithAddr(IIC_ADDR & 0xFE, buf, (12+2+4));
		
	//if(ret == 1) {while(1){;}}
	/*
	if(!ret)
	{
	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(3);UART_SEND(0xff);UART_SEND(0xff);
	for(i=0;i<18;i++)
	{
	
	UART_SEND(buf[i]);
	}
	}*/
	//---------- 读取结果------------------------------------
  delay_nms(500);
	ret = IIC_ReadWithAddr(IIC_ADDR | 0x1, buf, (14 + 4));
	//if(ret == 1) {while(1){;}}
	/*
	if(!ret)
	{
	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(6);UART_SEND(0xff);UART_SEND(0xff);
	for(i=0;i<18;i++)
	{
	
	UART_SEND(buf[i]);
	}
	}*/
	//----------- 整理结果--5,11---------------------------------		
	gx_checksum_get=buf[3+4];  gx_checksum_get <<=8; gx_checksum_get+=buf[7+4];
	buf[3+4] =buf[12+4]; buf[7+4]=buf[12+4+1]; 
	for(i=0;i<12;i++)
			{buf[i] = buf[i+4];}
	checksum_ext();
	//UART_SEND(gx_checksum_get);UART_SEND(gx_checksum_get>>8);
	//UART_SEND(0xff);UART_SEND(0xff);
	//UART_SEND(gx_checksum);UART_SEND(gx_checksum>>8);
	//---------- 校验结果 ---------------------------
	gfail=0;
	if(gx_checksum_get != gx_checksum){gfail=1;}
		 
	//---------- 伪校验 ----------------------------------------
	fake_check();
	delay_nms(300);
	
	fake_check();
	delay_nms(300);
	fake_check();
	delay_nms(300);
	fake_check();
	delay_nms(300);
	
	//-----------------------------------------------------------
	  //--- uart send (fail) here ---------------------------------	
	UART_SEND(0xFF);UART_SEND(0xFF);
	UART_SEND(gfail);
	UART_SEND(0xFF);UART_SEND(0xFF);
//------------------------------------------------------------------------

/*	delay_nms(200);
	i2c_single_byte_write(0xca,0x00,gfail);
	
	i2c_single_byte_write(0xca,0x01,2);
	i2c_single_byte_write(0xca,0x02,3);
	i2c_single_byte_write(0xca,0x03,4);
	UART_SEND(i2c_single_byte_read(0xca,01));
	UART_SEND(i2c_single_byte_read(0xca,02));
	UART_SEND(i2c_single_byte_read(0xca,03));*/
//-------------------fpga 加密-----------------------

	accu_SMEC_GetRandom();
	//if(ret == 1) {while(1){;}}
	//--------------------------------------
	//---- 生成随机数存在buf[0..11]--------
  for(i=0;i<12;i++)		
			{k=buf[2+i];sd_delay(k);buf[i]=TL0;}

	//------ 用求和加密 ------------------
	if(gfail == 0)	{checksum_ext();}
	
  //----- 打包发送 3 7-------------------
 
  
	buf[12]=buf[3]; buf[3]=gx_checksum>>8; 
	buf[12+1]=buf[7]; buf[7]=gx_checksum;
	buf[12+1+1]=0;
	//UART_SEND(0xff);UART_SEND(0xff);UART_SEND(5);UART_SEND(0xff);UART_SEND(0xff);
	for(i=0;i<15;i++)
	{ 
	i2c_single_byte_write(0xca, i, buf[i]);
	//UART_SEND(buf[i]);
	}
		
	i2c_single_byte_write(0xca, 14, 0);
	//if(ret == 1) {while(1){;}}
	delay_nms(500);
	
	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(6);UART_SEND(0xff);UART_SEND(0xff);
	for(i=0;i<14;i++)
	{ 
	buf[i] = i2c_single_byte_read(0xca, i);
	UART_SEND(buf[i]);
	}
	//if(ret == 1) {while(1){;}}
	gx_checksum_get=buf[3];  gx_checksum_get <<=8; gx_checksum_get+=buf[7];
	buf[3] =buf[12]; buf[7]=buf[12+1]; 
	
	if(flag%2 == 0){checksum_ext();}
	UART_SEND(gx_checksum_get);UART_SEND(gx_checksum_get>>8);
	UART_SEND(0xff);UART_SEND(0xff);
	UART_SEND(gx_checksum);UART_SEND(gx_checksum>>8);
	//---------- 校验结果 ---------------------------
	
	if(gx_checksum_get != gx_checksum){gfail=1;}
	//--- uart send (fail) here ---------------------------------
	UART_SEND(0xFF);UART_SEND(0xFF);	
	UART_SEND(gfail);
	UART_SEND(0xFF);UART_SEND(0xFF);
	
	if(gfail == 0)
	{
//------------------------------------------change bit
	buf[15]=buf[1];buf[16]=buf[5];buf[17]=buf[12];buf[18]=buf[2];buf[19]=buf[6];buf[20]=buf[13];           		//1<--->9
	buf[12]=buf[0];buf[1]=buf[9];buf[5]=buf[4];buf[2]=buf[11];buf[6]=buf[8];buf[13]=buf[10];					//5<--->4
	buf[0]=buf[17];buf[9]=buf[15];buf[4]=buf[16];buf[11]=buf[18];buf[8]=buf[19];buf[10]=buf[20];        		//12<--->10
	buf[14]=0;buf[3] =gx_checksum_get>>8; buf[7]=gx_checksum_get; 												//2<--->11
	}
	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(7);UART_SEND(0xff);UART_SEND(0xff);								//6<--->8
	for(i=0;i<15;i++)																							//13<--->10
	{ 
	i2c_single_byte_write(0xca, i, buf[i]);
	UART_SEND(buf[i]);
	}
	///
//	UART_SEND(0xff);UART_SEND(0xff);UART_SEND(8);UART_SEND(0xff);UART_SEND(0xff);
//	for(i=0;i<14;i++)
//	{ 
//	buf[i] = i2c_single_byte_read(0xca, i);
//	UART_SEND(buf[i]);
//	}
	delay_nms(200);
	i2c_single_byte_write(0xca, 14, 1);
	
	delay_nms(200);
    }
	//else ;
	}
}
 






//-----------------------------------------

