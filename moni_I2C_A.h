
#ifndef uchar 
#define uchar unsigned char 
#endif 
#ifndef uint 
#define uint unsigned int 
#endif 


unsigned char i2c_single_byte_read(unsigned char xdevaddr,unsigned char RdAddr);
void  i2c_single_byte_write(unsigned char xdevaddr,unsigned char xbyte_addr,unsigned char xbyte_data);
//uchar read_i2c_slave_si(uchar xdevaddr,uchar RdAddr);
//void  set_i2c_slave_si(uchar xdevaddr,uchar xbyte_addr,uchar xbyte_data);