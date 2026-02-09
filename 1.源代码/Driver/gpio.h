#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h"

#define KEY1 PBin(12)
#define KEY2 PBin(13)
#define KEY3 PBin(14)

#define FLAME  PBin(15)
#define BEEP   PCout(13)
#define FAN    PCout(14)
#define RELAY  PCout(15)
#define MOTOR  PAout(0)

void KEY_GPIO_Init(void);//Òý½Å³õÊ¼»¯
void MOTOR_GPIO_Init(void);

#endif
