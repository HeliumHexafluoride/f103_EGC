#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f1xx_hal.h"
#include "spi.h"

#define DIS_DIR  3  //调整显示方向，0--正常竖显、1--旋转90度显示、2--旋转180度显示、3--旋转270度显示
#define RED          0XF800	  //红色
#define GREEN        0X07E0	  //绿色
#define BLUE         0X001F	  //蓝色
#define WHITE        0XFFFF	  //白色
#define BLACK        0X0000  //黑色
#define YELLOW        	 0xFFE0  //黄色
#define BROWN 			     0X43BF //棕色
#define BRRED 			     0X07F8 //棕红色
#define GRAY  			     0X7BCF //灰色
#define DARKBLUE      	 0XFE30	//深蓝色
#define LIGHTBLUE      	 0X8283	//浅蓝色  
#define GRAYBLUE       	 0XABA7 //灰蓝色
#define LIGHTGREEN     	 0X7BE0 //浅绿色
#define LGRAY 			     0X39E7 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0X59AE //浅灰蓝色(中间层颜色)
#define LBBLUE           0XD4ED //浅棕蓝色(选择条目的反色)
#define DARKGREY     0x4208//深灰色

#define ECG_BACKGROUND_COLOR  BLACK    // 背景色 - 黑色
#define ECG_GRID_COLOR        DARKGREY // 网格线颜色 (例如 0x4208)
#define ECG_WAVEFORM_COLOR    GREEN    // 波形颜色 (例如 0x07E0)
#define ECG_TEXT_COLOR        WHITE    // 文本的前景色 - 白色 (注意：display_char16_16的背景是固定的白色)
#define ECG_HR_NUM_COLOR      YELLOW   // 心率数值的颜色 (前景)


// 1. 定义物理屏幕尺寸 (这是固定不变的)
#define LCD_PHYSICAL_WIDTH  240
#define LCD_PHYSICAL_HEIGHT 320

// 3. 根据 DIS_DIR 条件编译逻辑屏幕尺寸 TFT_COLUMN_NUMBER 和 TFT_LINE_NUMBER
#if (DIS_DIR == 0 || DIS_DIR == 2) // 竖屏模式 (DIS_DIR = 0 for Normal Portrait, DIS_DIR = 2 for Portrait Inverted)
    #define TFT_COLUMN_NUMBER LCD_PHYSICAL_WIDTH  // 逻辑宽度 = 物理宽度 (240)
    #define TFT_LINE_NUMBER   LCD_PHYSICAL_HEIGHT // 逻辑高度 = 物理高度 (320)
#elif (DIS_DIR == 1 || DIS_DIR == 3) // 横屏模式 (DIS_DIR = 1 for Landscape, DIS_DIR = 3 for Landscape Inverted)
    #define TFT_COLUMN_NUMBER LCD_PHYSICAL_HEIGHT // 逻辑宽度 = 物理高度 (320)
    #define TFT_LINE_NUMBER   LCD_PHYSICAL_WIDTH  // 逻辑高度 = 物理宽度 (240)
#else
    #error "Invalid DIS_DIR value! Please set DIS_DIR to 0, 1, 2, or 3."
#endif
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

// --- 布局参数定义 (字符宽度基于16x16字体，通常宽度是8像素) ---
#define CHAR_WIDTH_16PX       8 // 16x16字体的典型宽度 (需要根据你的chines_word字体确认)
#define CHAR_HEIGHT_16PX      16
// 波形显示区域
#define WAVE_START_X          0
#define WAVE_START_Y          CHAR_HEIGHT_16PX * 2 // 在标签下方
#define WAVE_HEIGHT           175
#define WAVE_BOTTOM_Y         WAVE_START_Y+WAVE_HEIGHT
#define WAVE_WIDTH            TFT_COLUMN_NUMBER
#define WAVE_BASELINE_Y       (WAVE_START_Y + WAVE_HEIGHT / 2)

#define BUFFERSIZE WAVE_HEIGHT*10*2

// 我们需要定义静态绘制特有的参数
#define STATIC_ECG_X_INCREMENT  3   // 每个ECG点在X轴上前进的像素数
#define STATIC_ECG_Y_SCALE      1   // Y轴缩放因子
#define MIN_ECG_DATA_VALUE      -2
#define MAX_ECG_DATA_VALUE      2
// 心率显示区域
#define HR_LABEL_X            10
#define HR_LABEL_Y            10
#define HR_VALUE_X            (HR_LABEL_X + 3 * CHAR_WIDTH_16PX) // "HR:" 占3个字符宽度
#define HR_VALUE_Y            HR_LABEL_Y

// 导联标签区域 (示例)
#define LEAD_LABEL_X          (TFT_COLUMN_NUMBER - (6 * CHAR_WIDTH_16PX) - 10) // "LEADII" 占6个字符，右边距10
#define LEAD_LABEL_Y          10
extern unsigned char  chines_word[  ][32];
extern unsigned char  picture_tab[PIC_NUM];
extern uint16_t  POINT_COLOR;
extern uint16_t  BACK_COLOR;

#define SAMPLE_ECG_LEN (sizeof(sample_ecg_pattern)/sizeof(sample_ecg_pattern[0]))

void TFT_SET_ADD(unsigned short int X,unsigned short int Y,unsigned short int X_END,unsigned short int Y_END);
void TFT_Clear(unsigned short int back_c);
void TFT_Init(void);
void display_char16_16(unsigned int x,unsigned int y,unsigned long color,unsigned char word_serial_number);
void Picture_Display(const unsigned char *ptr_pic);
// 来自第二个代码片段的SPI函数原型 (应在 "LCD.h" 中)
uint32_t TFT_Pow(uint8_t m, uint8_t n);
void TFT_Draw_Point(uint16_t x, uint16_t y, uint16_t color);
void TFT_Fill_Region(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);
void TFT_Draw_VLine(uint16_t x, uint16_t y_start, uint16_t y_end, uint16_t color) ;
void TFT_Draw_HLine(uint16_t y, uint16_t x_start, uint16_t x_end, uint16_t color);
void TFT_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode) ;
void TFT_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode) ;
void TFT_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t mode, uint8_t *p);
void TFT_Draw_Monochrome_Bmp(uint16_t x_start, uint16_t y_start, uint16_t w, uint16_t h, const uint8_t *bmp) ;
int16_t GetNextECGDataPoint(void);
void TFT_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ECG_UpdateWaveform(int16_t newDataPoint, int16_t minExpectedData, int16_t maxExpectedData);
void ECG_ShowNum_16x16(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t color);
void ECG_UpdateHeartRate(uint16_t hr_value);
void TFT_ShowChar(uint16_t x_start_pos, uint16_t y_start_pos, uint8_t ascii_char, uint8_t font_height, uint8_t mode);

void ECG_Screen_Init(); // 绘制ECG屏幕的静态布局

extern uint8_t LCD_buffer[BUFFERSIZE];

#endif //__LCD_H__
