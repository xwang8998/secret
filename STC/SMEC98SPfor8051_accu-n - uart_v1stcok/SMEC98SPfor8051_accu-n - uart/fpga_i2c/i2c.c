/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC15F4K60S4 ϵ�� ��ʱ��0��16λ�Զ���װ��ģʽ����---------------*/
/* --- Mobile: (86)13922805190 -------------- -------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966-------------------------*/
/* --- Web: www.STCMCU.com --------------------------------------------*/
/* --- Web: www.GXWMCU.com --------------------------------------------*/
/* ���Ҫ�ڳ�����ʹ�ô˴���,���ڳ�����ע��ʹ����STC�����ϼ�����        */
/* ���Ҫ��������Ӧ�ô˴���,����������ע��ʹ����STC�����ϼ�����        */
/*---------------------------------------------------------------------*/

//��ʾ����Keil������������ѡ��Intel��8058оƬ�ͺŽ��б���
//�����ر�˵��,����Ƶ��һ��Ϊ11.0592MHz


#include "reg51.h"
#include "intrins.h"
//#include	"moni_I2C_A.h"

//extern  set_i2c_slave(uchar xdevaddr,uchar xbyte_addr,uchar xbyte_data);
//-----------------------------------------------
typedef unsigned char BYTE;
typedef unsigned int WORD;
#define FOSC 11059200L

#define T1MS (65536-FOSC/1000)      //1Tģʽ
//#define T1MS (65536-FOSC/12/1000) //12Tģʽ

sfr AUXR = 0x8e;                    //Auxiliary register
sbit    FPGA_CTR = P3^4;
sbit	IIC_SDA	=	P3^3;//P2^7;
sbit	IIC_SCL	=	P3^2;//P2^6;

void display(unsigned char aa);
void delay(unsigned int t);
void delay_IIC(void);
void IIC_Init(void);
void IIC_start(void);
void IIC_stop(void);
bit IIC_Tack(void);
void IIC_single_byte_write(unsigned char Daddr,unsigned char Waddr,unsigned char Data);
unsigned char IIC_single_byte_read(unsigned char Daddr,unsigned char Waddr);
void IIC_write_byte(unsigned char Data);
unsigned char IIC_read_byte(void);
//-----------------------------------------------
int time = 0;
int s =0;
unsigned char math_data_H,math_data_L; 
	unsigned char end_data_H,end_data_L;
/* Timer0 interrupt routine */



//-----------------------------------------------
//========================== �����Ӻ���===============================
void delay(unsigned int t)
{// ��ʱ����
while(t){t--;}; //��ʱѭ������
}
//-------------------------------------------------------------------
void delay_IIC(void)
{//IIC ����������ʱ������
//�ú����ǿպ�������ʱ4 ���������ڡ�
;;
}
//-------------------------------------------------------------------
void IIC_Init(void)
{//IIC ���߳�ʼ������
IIC_SDA=1;// �ͷ�IIC ���ߵ������ߡ�
IIC_SCL=1;// �ͷ�IIC ���ߵ�ʱ���ߡ�
}
//-------------------------------------------------------------------
void IIC_start(void)
{//IIC ���߲�����ʼ�źź���
IIC_SDA=1;// ����������
IIC_SCL=1;// ����ʱ����
delay_IIC();
IIC_SDA=0;// ��ʱ����Ϊ�ߵ�ƽʱ�����������ߣ�������ʼ�źš�
delay_IIC();
IIC_SCL=0;// ����ʱ����
}
//-------------------------------------------------------------------
void IIC_stop(void)
{//IIC ���߲���ֹͣ�źź���
IIC_SDA=0;// ����������
delay_IIC();
IIC_SCL=1;// ����ʱ���ߡ�
delay_IIC();
IIC_SDA=1;// ʱ��ʱ��Ϊ�ߵ�ƽʱ�����������ߣ�����ֹͣ�źš�
delay_IIC();
}
//-------------------------------------------------------------------
bit IIC_Tack(void)
{// ����Ӧ���źź���
bit ack;// ����һ��λ���������ݴ�Ӧ��״̬��
IIC_SDA=1;// �ͷ��������ߣ�׼������Ӧ���źš�
delay_IIC();
IIC_SCL=1;// ����ʱ���ߡ�
delay_IIC();
ack=IIC_SDA;// ��ȡӦ���źŵ�״̬��
delay_IIC();
IIC_SCL=0;// ����ʱ���ߡ�
delay_IIC();
return ack;// ����Ӧ���źŵ�״̬�� 0 ��ʾӦ�� 1 ��ʾ��Ӧ��
}
//-------------------------------------------------------------------
void IIC_write_byte(unsigned char Data)
{// ��IIC ����д��һ���ֽڵ����ݺ���
unsigned char i;
for(i=0;i<8;i++)// ��8 λ����
{
IIC_SDA=Data&0x80;// д���λ������
delay_IIC();
IIC_SCL=1; // ����ʱ���ߣ�����д�뵽�豸�С�
delay_IIC();
IIC_SCL=0;// ����ʱ���ߣ�����ı������ߵ�״̬
delay_IIC();
Data=Data<<1;// ��������һλ���Ѵθ�λ�������λ,Ϊд��θ�λ��׼��
}
}
//-------------------------------------------------------------------
unsigned char IIC_read_byte()
{// ��IIC ���߶�ȡһ���ֽڵ����ݺ���
unsigned char i;
unsigned char Data; //����һ������Ĵ�����
for(i=0;i<8;i++)// ��8 λ����
{
IIC_SCL=1;// ����ʱ���ߣ�Ϊ��ȡ��һλ������׼����
delay_IIC();
Data=Data<<1;// �������ֽڵ���������һλ��׼����ȡ���ݡ�
delay_IIC();
if(IIC_SDA)// ���������Ϊ��ƽ��ƽ��
Data=Data|0x1;// ��������ֽڵ����λд1��
IIC_SCL=0;// ����ʱ���ߣ�Ϊ��ȡ��һλ������׼����
delay_IIC();
}
return Data;// ���ض�ȡ��һ���ֽ����ݡ�
}
//-------------------------------------------------------------------
void IIC_single_byte_write(unsigned char Daddr,unsigned char Waddr,unsigned char Data)
{// �������ַд��һ���ֽ����ݺ���
IIC_start();// ������ʼ�ź�
IIC_write_byte(Daddr);// д���豸��ַ��д��
FPGA_CTR=0;
IIC_Tack();// �ȴ��豸��Ӧ��
FPGA_CTR=1;
IIC_write_byte(Waddr);// д��Ҫ�����ĵ�Ԫ��ַ��
FPGA_CTR=0;
IIC_Tack();// �ȴ��豸��Ӧ��
FPGA_CTR=1;
IIC_write_byte(Data);// д�����ݡ�
FPGA_CTR=0;
IIC_Tack();// �ȴ��豸��Ӧ��
FPGA_CTR=1;
IIC_stop();// ����ֹͣ���š�
}
//-------------------------------------------------------------------
unsigned char IIC_single_byte_read(unsigned char Daddr,unsigned char Waddr)
{// �������ַ��ȡһ���ֽ����ݺ���
unsigned char Data;//����һ������Ĵ�����
IIC_start();// ������ʼ�ź�
IIC_write_byte(Daddr);// д���豸��ַ��д��
IIC_Tack();// �ȴ��豸��Ӧ��
IIC_write_byte(Waddr);// д��Ҫ�����ĵ�Ԫ��ַ��
IIC_Tack();// �ȴ��豸��Ӧ��
IIC_start();// ������ʼ�ź�
IIC_write_byte(Daddr+1);// д���豸��ַ������ ��
IIC_Tack();// �ȴ��豸��Ӧ��
Data=IIC_read_byte();// д�����ݡ�
IIC_Tack();
IIC_stop();// ����ֹͣ���š�

//------------------- ���ض�ȡ������--------------------
return Data;// ���ض�ȡ��һ���ֽ����ݡ�
}
//-------------------------------------------------------------------
void Delay1000ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 43;
	j = 6;
	k = 203;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

//--------------------------------------------
