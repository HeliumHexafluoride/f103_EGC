#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f1xx_hal.h"
#include "spi.h"

#define DIS_DIR  0  //调整显示方向，0--正常竖显、1--旋转90度显示、2--旋转180度显示、3--旋转270度显示
#define     RED          0XF800	  //红色
#define     GREEN        0X07E0	  //绿色
#define     BLUE         0X001F	  //蓝色
#define     WHITE        0XFFFF	  //白色
#define     BLACK        0X0000  //黑色

#define TFT_COLUMN_NUMBER 240
#define TFT_LINE_NUMBER 320
#define TFT_COLUMN_OFFSET 0

#define PIC_NUM 28800			//图片数据大小

#define TFT_RST_0  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET)       // 设置RST接口到PC7
#define TFT_RST_1  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
#define TFT_DC_0   HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);      // 设置DC接口到PC8
#define TFT_DC_1   HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
#define TFT_CS_0   HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);    // 设置CS接口到PC6
#define TFT_CS_1   HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
#define TFT_BL_0   HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);    // 设置BL接口到PC9
#define TFT_BL_1   HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);


extern unsigned char  chines_word[  ][32];
extern unsigned char  picture_tab[PIC_NUM];

void TFT_SET_ADD(unsigned short int X,unsigned short int Y,unsigned short int X_END,unsigned short int Y_END);
void TFT_Clear(unsigned short int back_c);
void TFT_Init(void);
void display_char16_16(unsigned int x,unsigned int y,unsigned long color,unsigned char word_serial_number);
void Picture_Display(const unsigned char *ptr_pic);




#endif //__LCD_H__
