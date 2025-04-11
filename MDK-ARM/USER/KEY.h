#ifndef KEY_H
#define KEY_H

#include "main.h"

//  引脚定义
#define    KEY1_GPIO_PORT    GPIOC			   
#define    KEY1_GPIO_PIN		 GPIO_PIN_3

#define    KEY2_GPIO_PORT    GPIOC		   
#define    KEY2_GPIO_PIN		  GPIO_PIN_2

#define    KEY3_GPIO_PORT    GPIOC		   
#define    KEY3_GPIO_PIN		  GPIO_PIN_1

#define    KEY4_GPIO_PORT    GPIOC		   
#define    KEY4_GPIO_PIN		  GPIO_PIN_0



 /** 按键按下标置宏
	*  按键按下为高电平，设置 KEY_ON=1， KEY_OFF=0
	*  若按键按下为低电平，把宏设置成KEY_ON=0 ，KEY_OFF=1 即可
	*/
#define KEY_ON	0
#define KEY_OFF	1

#endif