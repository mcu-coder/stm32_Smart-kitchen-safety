#ifndef __DS18B20_H
#define __DS18B20_H 
#include "sys.h"   
   	
//如果想要修改引脚，只需修改下面的宏
#define RCC_DS18B20_PORT 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define DS18B20_GPIO_PIN      GPIO_Pin_11
#define DS18B20_GPIO_PORT     GPIOA

//IO方向设置（CRL寄存器对应引脚0~7,CRH寄存器对应引脚8~15）
//DS18B20_GPIO_PORT->CRH&=0xFFFFFFF0为PA8引脚输出模式对应的寄存器清空
//DS18B20_GPIO_PORT->CRH|=0x00000008将(CNF8[1:0]设置为10:上拉/下拉输入模式，MODE8[1;0]设置为00:输入模式)
//DS18B20_GPIO_PORT->CRH|=0x00000003将(CNF8[1:0]设置为00:通用推挽输出模式 ，MODE8[1;0]设置为11:最大50MHZ)
#define DS18B20_IO_IN()  {DS18B20_GPIO_PORT->CRH&=0xFFFF0FFF;DS18B20_GPIO_PORT->CRH|=0x00008000;}
#define DS18B20_IO_OUT() {DS18B20_GPIO_PORT->CRH&=0xFFFF0FFF;DS18B20_GPIO_PORT->CRH|=0x00003000;}

#define DS18B20_OUT_0   GPIO_ResetBits(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)//IO为低电平
#define DS18B20_OUT_1   GPIO_SetBits(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)//IO为高电平
#define READ_DS18B20_IO GPIO_ReadInputDataBit(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN)//读取IO电平
		
void DS18B20_GPIO_Init(void);
u8 DS18B20_Init(void);			//初始化DS18B20
float ReadTemperature(void); //获取温度值

#endif

















