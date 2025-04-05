#include "key.h"




u8 KEY_TYPE;

//按键及电源控制初始化
void KEY1_Init(void) //IO初始化
{ 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);//使能PORTA/B时钟

	//PA0
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//下拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	
	//PA1
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_1);	
	
	//PB8、PB9
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8|GPIO_Pin_9;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9);		
}

uint8_t key_down = 0;			//按键按下标志位

//按键扫描
uint8_t key_scan(void)
{
	uint8_t key_res_temp = 0;
	if(KEY1 == 0 && key_down == 0)
	{
		key_down = 1;
		key_res_temp = 1;
	}
	if(KEY2 == 0 && key_down == 0)
	{
		key_down = 1;
		key_res_temp = 2;
	}
	if(KEY3 == 0 && key_down == 0)
	{
		key_down = 1;
		key_res_temp = 3;
	}
	if(KEY4 == 1 && key_down == 0)
	{
		key_down = 1;
		key_res_temp = 4;
	}
	if(KEY1 == 1 && KEY2 == 1 && KEY3 == 1 && KEY4 == 0)
	{
		key_down = 0;
	}
	return key_res_temp;
}





			    


