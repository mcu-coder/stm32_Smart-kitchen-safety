#include "ds18b20.h"
#include "delay.h"	

/*******************************************************************************
函数名：DS18B20_GPIO_Init
功能：初始化DS18B20引脚
输入：
输出：
返回值：
备注：
*******************************************************************************/
void DS18B20_GPIO_Init(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
 	
		RCC_APB2PeriphClockCmd(RCC_DS18B20_PORT, ENABLE);	 //使能PORTA口时钟 
		
		GPIO_InitStructure.GPIO_Pin = DS18B20_GPIO_PIN;				
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStructure);

		GPIO_SetBits(DS18B20_GPIO_PORT,DS18B20_GPIO_PIN);    //输出1
}

/*******************************************************************************
函数名：DS18B20_Init
功能：初始化DS18B20
输入：
输出：
返回值：初始化成功为0，不成功为1
备注：
*******************************************************************************/
u8 DS18B20_Init(void)
{
	  unsigned char wait=0;
	    
		DS18B20_IO_OUT(); //输出模式
	  DS18B20_OUT_0;     //拉低
	  delay_us(750);     //至少延时480us
	  DS18B20_OUT_1;     //拉高 
	  delay_us(15);      //15us
	  DS18B20_IO_IN();  //输入模式
	  while(READ_DS18B20_IO && wait++<200)delay_us(1);//等待高电平结束
	  if(wait>=200)return 1;
	  else wait=0;
	  while(!READ_DS18B20_IO && wait++<240)delay_us(1);//等待低电平结束
	  if(wait>=240)return 1;
	  else return 0; 
}

/*******************************************************************************
函数名：DS18B20_ReadByte
功能：从DS18B20读一个字节
输入：
输出：
返回值：读取到的字节
备注：
*******************************************************************************/
unsigned char DS18B20_ReadByte(void)
{
		unsigned char i;
    unsigned char dat = 0;

    for (i=0; i<8; i++)             //8位计数器
    {
        dat >>= 1;                 
			  DS18B20_IO_OUT();           //输出模式
        DS18B20_OUT_0;              //开始时间片
        delay_us(2);                //延时等待
        DS18B20_OUT_1;              //准备接收
			  DS18B20_IO_IN();            //输入模式
        delay_us(12);                //接收延时
        if(READ_DS18B20_IO) dat |= 0x80;   //读取数据 
        delay_us(60);               //等待时间片结束
    }

    return dat;
}

/*******************************************************************************
函数名：DS18B20_WriteByte
功能：写一个字节
输入：unsigned char dat
输出：
返回值：
备注：
*******************************************************************************/
void DS18B20_WriteByte(unsigned char dat)
{
	  unsigned char i;
	  unsigned char temp;
	
		DS18B20_IO_OUT();//输出模式
    for (i=1; i<=8; i++) 
		{
					temp = dat & 0x01;  
					dat  = dat >> 1;
					if (temp) 
					{
							DS18B20_OUT_0;
							delay_us(2);                            
							DS18B20_OUT_1; //写1
							delay_us(60);             
					}
					else 
					{
							DS18B20_OUT_0;//写0
							delay_us(60);             
							DS18B20_OUT_1;
							delay_us(2);                          
					}		
		}
}

/**************************************
从DS18B20中获取温度值得浮点值
参数: 空
返回值:  读取到的温度值(有效范围-55.0~125.0)
**************************************/
float ReadTemperature(void)
{
    unsigned char TPH;							//存放温度值的高字节
    unsigned char TPL;							//存放温度值的低字节
    short i16=0;
    float f32=0;

	  DS18B20_Init();
    DS18B20_WriteByte(0xCC);  //跳过ROM命令
    DS18B20_WriteByte(0x44);   //开始转换命令
    DS18B20_Init();
    DS18B20_WriteByte(0xCC);        //跳过ROM命令
    DS18B20_WriteByte(0xBE);        //读暂存存储器命令
    TPL = DS18B20_ReadByte();       //读温度低字节
    TPH = DS18B20_ReadByte();       //读温度高字节
    i16 = 0;
    i16 = (TPH<<8) |TPL;				// 将高位(MSB)与低位(LSB)合并
    f32 = i16 * 0.0625;				// 12bit精度时温度值计算
    return(f32);					// 返回读取到的温度数值(float型)
}


