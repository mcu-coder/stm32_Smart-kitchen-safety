#include "sys.h"
#include "delay.h"
#include "adc.h"
#include "gpio.h"
#include "OLED_I2C.h"
#include "ds18b20.h"
#include "timer.h"
#include "usart1.h"
#include "esp8266.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern const unsigned char BMP[];

char  display[16];             //显示缓存区
short temperature=0;           //温度
u8    setTempValue  = 40;     //温度上限
u8    setSmokeValue = 60;     //烟雾上限
u8    alarmFlag = 0x00;      //蜂鸣器报警标志
u16   smoke=0;               //烟雾
u8    setn=0;                //记录设置按键按下的次数

u8 PWM=5;                   //PWM调节值，用于控制舵机

bool shanshuo=0;
bool shuaxin=0;
bool sendFlag = 1;
bool flameFlag=0;

void displayInitInterface(void)    //显示初始页面
{
		u8 i;
	
	  for(i = 0;i < 6;i ++)OLED_ShowCN(i*16+16,0,i+0,0);    //显示中文: 厨房安全检测
	  for(i = 0;i < 2;i ++)OLED_ShowCN(i*16,4,i+6,0);       //显示中文: 温度
	  for(i = 0;i < 2;i ++)OLED_ShowCN(i*16,6,i+8,0);      //显示中文: 烟雾
	  OLED_ShowChar(32,4,':',2,0);
		OLED_ShowChar(32,6,':',2,0);
}

void Get_Temperature(void)       //获取温度
{
		temperature=ReadTemperature();
	
	  if(temperature>=setTempValue)
		 {
				if(!(alarmFlag&0x01))
				{
						alarmFlag|=0x01;
					  shanshuo = 0;
				}
		 }
		 else
		 {
				alarmFlag&=0xFE;
		 }
	
	  if(temperature>=setTempValue && shanshuo)
		{
				OLED_ShowStr(40, 4,"      ", 2,0);
			  
		}
		else
		{
				sprintf(display," %d",temperature);
				OLED_ShowStr(40, 4, (u8*)display, 2,0);//显示温度
			  OLED_ShowCentigrade(68, 4);   //显示摄氏度
		}
}

void Get_Smoke(void)    //获取烟雾浓度
{
		u16 test_adc=0;
	
	  /* 获取烟雾浓度 */
	  test_adc = Get_Adc_Average(ADC_Channel_9,10);//读取通道9的10次AD平均值
		smoke = test_adc*99/4096;//转换成0-99百分比
	
	  if(smoke>=setSmokeValue)
		 {
				if(!(alarmFlag&0x02))
				{
						alarmFlag|=0x02;
					  shanshuo = 0;
				}
		 }
		 else
		 {
				alarmFlag&=0xFD;
		 }
	
	  if(smoke>=setSmokeValue && shanshuo)
		{
				OLED_ShowStr(40, 6,"      ", 2,0);
		}
		else
		{
				sprintf(display," %02d %%",smoke);
				OLED_ShowStr(40, 6, (u8*)display, 2,0);//显示温度
		}
}

void displaySetValue(void) //显示设置值
{
	  if(setn==1)
		{
				OLED_ShowChar(56,4,setTempValue%100/10+'0',2,0);//显示
				OLED_ShowChar(64,4,setTempValue%10+'0',2,0);//显示
		}
		if(setn==2)
		{
			  OLED_ShowChar(56,4,setSmokeValue%100/10+'0',2,0);//显示
				OLED_ShowChar(64,4,setSmokeValue%10+'0',2,0);//显示
			  OLED_ShowChar(72,4,'%',2,0);  
		}	
}

void keyscan(void)   //按键扫描
{
	 u8 i;
	
	 if(KEY1 == 0) //设置键
	 {
			delay_ms(20);
		  if(KEY1 == 0)
			{
					while(KEY1 == 0);
				  setn ++;
				  if(setn == 1)
					{
							OLED_CLS();//清屏
							for(i = 0;i < 4;i ++)OLED_ShowCN(i*16+32,0,i+10,0);//显示中文：设置温度
						  OLED_ShowCentigrade(75, 4);   //显示摄氏度
					}
					if(setn == 2)
					{
							for(i = 0;i < 4;i ++)OLED_ShowCN(i*16+32,0,i+14,0);//显示中文：设置烟雾
						  OLED_ShowChar(80,4,' ',2,0);  
					}
					if(setn >= 3)
					{
							setn = 0;
						  OLED_CLS();//清屏
						  displayInitInterface();
					}
					displaySetValue();
			}
	 }
	 if(KEY2 == 0) //加键
	 {
			delay_ms(80);
		  if(KEY2 == 0)
			{
					if(setTempValue  < 99 && setn==1)setTempValue++;
				  if(setSmokeValue < 99 && setn==2)setSmokeValue++;
					displaySetValue();
			}
	 }
	 if(KEY3 == 0) //减键
	 {
			delay_ms(80);
		  if(KEY3 == 0)
			{
				  if(setTempValue  > 0 && setn==1)setTempValue--;
				  if(setSmokeValue > 0 && setn==2)setSmokeValue--;
					displaySetValue();
			}
	 }
}

void UsartSendReceiveData(void)
{
		unsigned char *dataPtr = NULL;
	  char *str1=0,i;
	  int  setValue=0;
	  char setvalue[3]={0};
	  char SEND_BUF[30];
	
	  dataPtr = ESP8266_GetIPD(0);   //接收数据
	  if(dataPtr != NULL)
		{
			  if(strstr((char *)dataPtr,"temp:")!=NULL)
				{
					  BEEP = 1;
						delay_ms(80);
					  BEEP = 0;
					
						str1 = strstr((char *)dataPtr,"temp:");
					  
					  while(*str1 < '0' || *str1 > '9')        //判断是不是0到9有效数字
						{
								str1 = str1 + 1;
								delay_ms(10);
						}
						i = 0;
						while(*str1 >= '0' && *str1 <= '9')        //判断是不是0到9有效数字
						{
								setvalue[i] = *str1;
								i ++; str1 ++;
								if(*str1 == ',')break;            //换行符，直接退出while循环
								delay_ms(10);
						}
						setvalue[i] = '\0';            //加上结尾符
						setValue = atoi(setvalue);
						if(setValue>=0 && setValue<=99)
						{
								setTempValue = setValue;    //设置的温度值
							  displaySetValue();
						}
				}	
				
			  if(strstr((char *)dataPtr,"smoke:")!=NULL)
				{
						str1 = strstr((char *)dataPtr,"smoke:");
					  
					  while(*str1 < '0' || *str1 > '9')        //判断是不是0到9有效数字
						{
								str1 = str1 + 1;
								delay_ms(10);
						}
						i = 0;
						while(*str1 >= '0' && *str1 <= '9')        //判断是不是0到9有效数字
						{
								setvalue[i] = *str1;
								i ++; str1 ++;
								if(*str1 == '\r')break;            //换行符，直接退出while循环
								delay_ms(10);
						}
						setvalue[i] = '\0';            //加上结尾符
						setValue = atoi(setvalue);
						if(setValue>=0 && setValue<=99)
						{
								setSmokeValue = setValue;    //设置的烟雾值
							  displaySetValue();
						}
				}	
				
			  ESP8266_Clear();									//清空缓存
		}
		if(sendFlag==1)    //1秒钟上传一次数据
		{
			  sendFlag = 0;		
			   
				memset(SEND_BUF,0,sizeof(SEND_BUF));   			//清空缓冲区
				sprintf(SEND_BUF,"$temp:%d#,$smoke:%d#",temperature,smoke);
        
			  if(flameFlag)
				{
						strcat(SEND_BUF,"flame");
				}
        
			  ESP8266_SendData((u8 *)SEND_BUF, strlen(SEND_BUF));
			  ESP8266_Clear();
		}
}

int main(void)
{
		delay_init();	           //延时函数初始化	 
    NVIC_Configuration();	   //中断优先级配置
	  I2C_Configuration();     //IIC初始化
	  delay_ms(200); 
	  OLED_Init();             //OLED液晶初始化
	  OLED_CLS();              //清屏
	   
	  displayInitInterface(); //显示初始界面
	  
		while(1)
		{ 
			   keyscan();  //按键扫描
		 
				 UsartSendReceiveData();
			   delay_ms(10);
		}
}
 

