#include "LCD.h"
#include "lcdfont_dma.h"



uint8_t LCD_buffer[BUFFERSIZE]={0};
//LCD的画笔颜色和背景色	   
// --- 颜色定义 ---

uint16_t POINT_COLOR;
uint16_t BACK_COLOR;



unsigned char  chines_word[ ][32]=   //汉字码
{
0x00,0x00,0xE4,0x3F,0x28,0x20,0x28,0x25,0x81,0x08,0x42,0x10,0x02,0x02,0x08,0x02,
0xE8,0x3F,0x04,0x02,0x07,0x07,0x84,0x0A,0x44,0x12,0x34,0x62,0x04,0x02,0x00,0x02,/*"深",0*/

0x88,0x20,0x88,0x24,0x88,0x24,0x88,0x24,0x88,0x24,0xBF,0x24,0x88,0x24,0x88,0x24,
0x88,0x24,0x88,0x24,0x88,0x24,0xB8,0x24,0x87,0x24,0x42,0x24,0x40,0x20,0x20,0x20,/*"圳",1*/

0x80,0x00,0x80,0x00,0x40,0x01,0x20,0x02,0x10,0x04,0x08,0x08,0xF4,0x17,0x83,0x60,
0x80,0x00,0xFC,0x1F,0x80,0x00,0x88,0x08,0x90,0x08,0x90,0x04,0xFF,0x7F,0x00,0x00,/*"金",2*/

0x80,0x00,0x82,0x00,0x84,0x0F,0x44,0x08,0x20,0x04,0xF0,0x3F,0x27,0x22,0x24,0x22,
0xE4,0x3F,0x04,0x05,0x84,0x0C,0x84,0x54,0x44,0x44,0x24,0x78,0x0A,0x00,0xF1,0x7F,/*"逸",3*/

0xF8,0x0F,0x08,0x08,0xF8,0x0F,0x08,0x08,0xF8,0x0F,0x00,0x00,0xFC,0x3F,0x04,0x00,
0xF4,0x1F,0x04,0x00,0xFC,0x7F,0x94,0x10,0x14,0x09,0x12,0x06,0x52,0x18,0x31,0x60,/*"晨",4*/

0x80,0x00,0x80,0x00,0x80,0x00,0xFC,0x1F,0x84,0x10,0x84,0x10,0x84,0x10,0xFC,0x1F,
0x84,0x10,0x84,0x10,0x84,0x10,0xFC,0x1F,0x84,0x50,0x80,0x40,0x80,0x40,0x00,0x7F,/*"电",5*/

0x00,0x00,0xFE,0x1F,0x00,0x08,0x00,0x04,0x00,0x02,0x80,0x01,0x80,0x00,0xFF,0x7F,
0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0xA0,0x00,0x40,0x00,/*"子",6*/
};





uint16_t ecg_waveform_buffer[WAVE_WIDTH];
int current_ecg_x_position = 0;               // 当前绘制新数据点的X轴位置 (相对于WAVE_START_X的偏移)
uint16_t last_ecg_y_value = WAVE_BASELINE_Y;  // 存储上一个数据点的Y值，用于连接成线

// --- 模拟数据 (当使用真实ADS1292R数据时请移除) ---
int simulated_hr = 75;
int ecg_data_idx = 0;

void SPI_SendByte(u8 Txdata)				//向液晶屏写一个8位数据
{
  while(HAL_SPI_GetState(&hspi2)!=HAL_SPI_STATE_READY){};
  // HAL_SPI_Transmit(&hspi2,&Txdata,1,100);
  HAL_SPI_Transmit_DMA(&hspi2,&Txdata,1);
	// HAL_SPI_TransmitReceive(&hspi2,&Txdata,&Rxdata,1,100);

  
}

void SPI_Send_MULTI_Byte(u8* Txdata,uint32_t size)				//向液晶屏写多个8位数据
{
  while(HAL_SPI_GetState(&hspi2)!=HAL_SPI_STATE_READY){};
  // HAL_SPI_Transmit(&hspi2,&Txdata,1,100);
  HAL_SPI_Transmit_DMA(&hspi2,Txdata,size);
	// HAL_SPI_TransmitReceive(&hspi2,&Txdata,&Rxdata,1,100);
  while(HAL_SPI_GetState(&hspi2)!=HAL_SPI_STATE_READY){};
  
}

void TFT_SEND_CMD(unsigned char o_command)
  {
    
    TFT_CS_0;
	TFT_DC_0;
    u8 temp=o_command;
    SPI_SendByte(temp);
    TFT_CS_1;
   
    //TFT_DC_1;
  }
  
//向液晶屏写一个8位数据
void TFT_SEND_DATA(unsigned char  o_data)
  { 
	TFT_CS_0;
    TFT_DC_1;
    SPI_SendByte(o_data);
    TFT_CS_1;
    
   }

//向液晶屏写多个8位数据
void TFT_SEND_MULTI_DATA(uint8_t*  o_data,uint32_t size)
  { 
	TFT_CS_0;
    TFT_DC_1;
    SPI_Send_MULTI_Byte(o_data,size);
    TFT_CS_1;
    
   }

void TFT_SET_ADD(unsigned short int X,unsigned short int Y,unsigned short int X_END,unsigned short int Y_END)
{	

            //判断地址是否合法
    if((X_END>(TFT_COLUMN_NUMBER-1))|| (X>(TFT_COLUMN_NUMBER-1)) )      //横向向显示    //超出x地址范围
    {
        X_END = (TFT_COLUMN_NUMBER-1);
            //X = 0;
    }   
    if ((Y_END > (TFT_LINE_NUMBER-1) )||(Y > (TFT_LINE_NUMBER-1) ))       //超出Y范围
    {
            //Y = 0;                          //移动到第一行
            Y_END =  ( TFT_LINE_NUMBER-1);
                
    }
	TFT_SEND_CMD(0x2A);     //窗口地址横向                   
    TFT_SEND_DATA(X>>8);     
    TFT_SEND_DATA(X);//start column
	TFT_SEND_DATA(X_END>>8);     
    TFT_SEND_DATA(X_END);//start column
	TFT_SEND_CMD(0x2B);     //窗口地址纵向                   
    TFT_SEND_DATA(Y>>8);     
    TFT_SEND_DATA(Y);		//start row
    TFT_SEND_DATA(Y_END>>8);     
    TFT_SEND_DATA(Y_END);		//start row
    TFT_SEND_CMD(0x2C);     //写入数据   
}  


void TFT_Clear(unsigned short int back_c)									//清除液晶数据，刷黑
  {
    unsigned int ROW=0,column=0;
	TFT_SET_ADD(0x0000,0x0000,TFT_COLUMN_NUMBER-1,TFT_LINE_NUMBER-1);//纵向
    for(ROW=0;ROW<TFT_LINE_NUMBER;ROW++)             //ROW loop
      { 
          for(column=0;column<TFT_COLUMN_NUMBER;column++)  //column loop
            {         
				TFT_SEND_DATA(back_c>>8);
				TFT_SEND_DATA(back_c);
            }
      }
  }
void TFT_Init(void)        
  {
	TFT_RST_0;
	delay_ms(100);
	TFT_RST_1;
	delay_ms(120);
//-----------------------ST7789V Frame rate setting-----------------//
//************************************************
                TFT_SEND_CMD(0x3A);        //65k mode
                TFT_SEND_DATA(0x05);
                TFT_SEND_CMD(0xC5); 		//VCOM
                TFT_SEND_DATA(0x1A);
                TFT_SEND_CMD(0x36);                 // 屏幕显示方向设置
	  //调整显示方向  MY,MX,MV,ML,RGB,MH,X,X       row-order，column-order，row/column exchange，
	      #if (DIS_DIR == 0)                   //正常竖显
            TFT_SEND_DATA(0x00);
        #elif (DIS_DIR == 1)                   //旋转90度显示
            TFT_SEND_DATA(0xA0);
        #elif (DIS_DIR == 2)                   //旋转180度显示
            TFT_SEND_DATA(0xC0);
        #elif(DIS_DIR == 3)                   //旋转270度显示
            TFT_SEND_DATA(0x60);
        #endif      
                //-------------ST7789V Frame rate setting-----------//
                TFT_SEND_CMD(0xb2);		//Porch Setting
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x00);
                TFT_SEND_DATA(0x33);
                TFT_SEND_DATA(0x33);

                TFT_SEND_CMD(0xb7);			//Gate Control
                TFT_SEND_DATA(0x05);			//12.2v   -10.43v
                //--------------ST7789V Power setting---------------//
                TFT_SEND_CMD(0xBB);//VCOM
                TFT_SEND_DATA(0x3F);

                TFT_SEND_CMD(0xC0); //Power control
                TFT_SEND_DATA(0x2c);

                TFT_SEND_CMD(0xC2);		//VDV and VRH Command Enable
                TFT_SEND_DATA(0x01);

                TFT_SEND_CMD(0xC3);			//VRH Set
                TFT_SEND_DATA(0x0F);		//4.3+( vcom+vcom offset+vdv)

                TFT_SEND_CMD(0xC4);			//VDV Set
                TFT_SEND_DATA(0x20);				//0v

                TFT_SEND_CMD(0xC6);				//Frame Rate Control in Normal Mode
                TFT_SEND_DATA(0X01);			//111Hz

                TFT_SEND_CMD(0xd0);				//Power Control 1
                TFT_SEND_DATA(0xa4);
                TFT_SEND_DATA(0xa1);

                TFT_SEND_CMD(0xE8);				//Power Control 1
                TFT_SEND_DATA(0x03);

                TFT_SEND_CMD(0xE9);				//Equalize time control
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x08);
                //---------------ST7789V gamma setting-------------//
                TFT_SEND_CMD(0xE0); //Set Gamma
                TFT_SEND_DATA(0xD0);
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x08);
                TFT_SEND_DATA(0x14);
                TFT_SEND_DATA(0x28);
                TFT_SEND_DATA(0x33);
                TFT_SEND_DATA(0x3F);
                TFT_SEND_DATA(0x07);
                TFT_SEND_DATA(0x13);
                TFT_SEND_DATA(0x14);
                TFT_SEND_DATA(0x28);
                TFT_SEND_DATA(0x30);
                 
                TFT_SEND_CMD(0XE1); //Set Gamma
                TFT_SEND_DATA(0xD0);
                TFT_SEND_DATA(0x05);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x09);
                TFT_SEND_DATA(0x08);
                TFT_SEND_DATA(0x03);
                TFT_SEND_DATA(0x24);
                TFT_SEND_DATA(0x32);
                TFT_SEND_DATA(0x32);
                TFT_SEND_DATA(0x3B);
                TFT_SEND_DATA(0x14);
                TFT_SEND_DATA(0x13);
                TFT_SEND_DATA(0x28);
                TFT_SEND_DATA(0x2F);

                TFT_SEND_CMD(0x20); 		//反显
	TFT_SEND_CMD(0x11); //Exit Sleep // 退出睡眠模式
	delay_ms(120); 
	TFT_SEND_CMD(0x29); //Display on // 开显示
  }
void display_char16_16(unsigned int x,unsigned int y,unsigned long color,unsigned char word_serial_number)
{
   unsigned int column;
  unsigned char tm=0,temp=0,xxx=0;

   TFT_SEND_CMD(0x2a);    //Column address set
  TFT_SEND_DATA(x>>8);    //start column
  TFT_SEND_DATA(x);
  x=x+15;
  TFT_SEND_DATA(x>>8);    //end column
  TFT_SEND_DATA(x);

  TFT_SEND_CMD(0x2b);     //Row address set
  TFT_SEND_DATA(y>>8);    //start row
  TFT_SEND_DATA(y); 
  y=y+15;
  TFT_SEND_DATA(y>>8);    //end row
  TFT_SEND_DATA(y);
    TFT_SEND_CMD(0x2C);     //Memory write
    
    
  for(column=0;column<32;column++)  //column loop
          {
        temp=chines_word[  word_serial_number ][xxx];
        for(tm=0;tm<8;tm++)
        {
        if(temp&0x01)
          {
				TFT_SEND_DATA(color>>8);
			  TFT_SEND_DATA(color);
          }
        else 
          {
			TFT_SEND_DATA(0XFF);
			  TFT_SEND_DATA(0XFF);
          }
          temp>>=1;
        }
        xxx++;
          
      }
}

void Picture_Display(const unsigned char *ptr_pic)
{
    unsigned long  number;
	unsigned int Ppdata;
	TFT_SEND_CMD(0x2a); 		//Column address set
	TFT_SEND_DATA(0x00); 		//start column
	TFT_SEND_DATA(0x00); 
	TFT_SEND_DATA(0x00);		//end column
	TFT_SEND_DATA(0x77);

	TFT_SEND_CMD(0x2b); 		//Row address set
	TFT_SEND_DATA(0x00); 		//start row
	TFT_SEND_DATA(0x00); 
	TFT_SEND_DATA(0x00);		//end row
	TFT_SEND_DATA(0x78);
	TFT_SEND_CMD(0x2C);			//Memory write
	  
	for(number=0;number<PIC_NUM;number++)	
          {
			Ppdata=*ptr_pic++;
            TFT_SEND_DATA(Ppdata);
          }
  }



// m^n函数 (无需修改，可直接使用)
uint32_t TFT_Pow(uint8_t m, uint8_t n) {
    uint32_t result = 1;
    while (n--) result *= m;
    return result;
}

// 已有的 TFT_DrawLine 函数，如果之前没加，现在加上
void TFT_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;

    while (1) {
        if (x1 < TFT_COLUMN_NUMBER && y1 < TFT_LINE_NUMBER) {
            TFT_Draw_Point(x1, y1, color);
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}
// 画点函数
// 入口数据：x, y   坐标
//           color  颜色
void TFT_Draw_Point(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= TFT_COLUMN_NUMBER || y >= TFT_LINE_NUMBER) return; // 边界检查
		TFT_SEND_CMD(0x2a); 		//Column address set
		TFT_SEND_DATA(x>>8); 		//start column
		TFT_SEND_DATA(x); 
		TFT_SEND_CMD(0x2b); 		//Row address set
		TFT_SEND_DATA(y>>8); 		//start row
		TFT_SEND_DATA(y); 
		TFT_SEND_CMD(0x2C);			//Memory write
		TFT_SEND_DATA(color>>8); 		//start row
		TFT_SEND_DATA(color);
}

// 在指定区域填充颜色 (移植自 Lcd_Clear)
// 入口数据：xsta,ysta   起始坐标
//           xend,yend   终止坐标
//           color       要填充的颜色
void TFT_Fill_Region(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color) {
    uint32_t i = 0;
    uint32_t num_pixels = 0;

    if (xsta > xend || ysta > yend) return; // 非法区域
    if (xsta >= TFT_COLUMN_NUMBER || ysta >= TFT_LINE_NUMBER) return;
    
    xend = (xend >= TFT_COLUMN_NUMBER) ? (TFT_COLUMN_NUMBER - 1) : xend;
    yend = (yend >= TFT_LINE_NUMBER) ? (TFT_LINE_NUMBER - 1) : yend;

    TFT_SET_ADD(xsta, ysta, xend, yend);
    num_pixels = (uint32_t)(xend - xsta + 1) * (yend - ysta + 1);

    if (num_pixels>(BUFFERSIZE/2)) return;

    for (i = 0; i < num_pixels; i++) {
        LCD_buffer[2*i]=color >> 8;
        LCD_buffer[2*i+1]=color & 0xFF;
    }
    TFT_SEND_MULTI_DATA(LCD_buffer,num_pixels*2);

    // for (i = 0; i < num_pixels; i++) {
    //     TFT_SEND_DATA(color >> 8);
    //     TFT_SEND_DATA(color & 0xFF);
    // }
}


// 绘制垂直线段 (移植自 Lcd_Show_Lin)
// 注意：原 Lcd_Show_Lin 的 y 坐标计算方式 (239-i) 很特殊，这里改为标准的从上到下。
// 如果需要原先的Y轴反转效果，需要在调用此函数前或在此函数内部调整Y坐标。
// 入口数据：x       线段的x坐标
//           y_start 线段的起始y坐标
//           y_end   线段的终止y坐标
//           color   颜色
void TFT_Draw_VLine(uint16_t x, uint16_t y_start, uint16_t y_end, uint16_t color) {
    uint16_t i;
    uint16_t start_y, end_y;

    if (x >= TFT_COLUMN_NUMBER) return; // 超出X范围

    // 确保 y_start <= y_end
    if (y_start <= y_end) {
        start_y = y_start;
        end_y = y_end;
    } else {
        start_y = y_end;
        end_y = y_start;
    }
    
    if (start_y >= TFT_LINE_NUMBER) return; // 整条线都在屏幕外
    end_y = (end_y >= TFT_LINE_NUMBER) ? (TFT_LINE_NUMBER - 1) : end_y;

    if (start_y > end_y) return; // 裁剪后无效

    TFT_SET_ADD(x, start_y, x, end_y);
    for (i = start_y; i <= end_y; i++) {
        TFT_SEND_DATA(color >> 8);
        TFT_SEND_DATA(color & 0xFF);
    }
}

// 绘制水平线段 (新增的，作为补充)
void TFT_Draw_HLine(uint16_t y, uint16_t x_start, uint16_t x_end, uint16_t color) {
    uint16_t i;
    uint16_t start_x, end_x;

    if (y >= TFT_LINE_NUMBER) return; // 超出Y范围

    if (x_start <= x_end) {
        start_x = x_start;
        end_x = x_end;
    } else {
        start_x = x_end;
        end_x = x_start;
    }

    if (start_x >= TFT_COLUMN_NUMBER) return; // 整条线都在屏幕外
    end_x = (end_x >= TFT_COLUMN_NUMBER) ? (TFT_COLUMN_NUMBER - 1) : end_x;
    
    if (start_x > end_x) return; // 裁剪后无效

    TFT_SET_ADD(start_x, y, end_x, y);
    for (i = start_x; i <= end_x; i++) {
        TFT_SEND_DATA(color >> 8);
        TFT_SEND_DATA(color & 0xFF);
    }
}




//显示字符的函数
void TFT_ShowChar(uint16_t x_start_pos, uint16_t y_start_pos, uint8_t ascii_char, uint8_t font_height, uint8_t mode)
{
    uint8_t  font_pixel_width;      // 字符的像素宽度
    uint8_t  bytes_per_col;         // 存储一列像素所需的字节数
    uint8_t  total_bytes_for_glyph; // 当前字符字模占用的总字节数
    const unsigned char *glyph_data_ptr; // 指向当前字符的字模数据

    uint16_t current_col_x;         // 当前正在绘制的屏幕列X坐标
    uint16_t current_pixel_y;       // 当前正在绘制的屏幕像素Y坐标
    uint8_t  byte_data_from_font;
    uint8_t  bit_idx_in_byte;

    // 1. 参数有效性检查和预处理
    if (x_start_pos >= TFT_COLUMN_NUMBER || y_start_pos >= TFT_LINE_NUMBER) {
        return; // 起始点就在屏幕外
    }

    // 空格字符处理
    if (ascii_char == ' ') {
        font_pixel_width = font_height / 2; // 空格宽度通常是字体高度的一半
        if (mode == 0) { // 不透明模式下，用背景色填充空格区域
            // 确保填充区域不超出屏幕
            uint16_t x_end_fill = x_start_pos + font_pixel_width - 1;
            uint16_t y_end_fill = y_start_pos + font_height - 1;
            if (x_end_fill >= TFT_COLUMN_NUMBER) x_end_fill = TFT_COLUMN_NUMBER - 1;
            if (y_end_fill >= TFT_LINE_NUMBER) y_end_fill = TFT_LINE_NUMBER - 1;

            if (x_start_pos <= x_end_fill && y_start_pos <= y_end_fill) { // 确保区域有效
                 TFT_Fill_Region(x_start_pos, y_start_pos, x_end_fill, y_end_fill, ECG_BACKGROUND_COLOR);
            }
        }
        return;
    }

    if (ascii_char < '!' || ascii_char > '~') { // 只处理从'!'到'~'的可见字符，空格已处理
        return;
    }

    uint8_t char_index_in_font = ascii_char - ' '; // 字体数据从空格开始索引

    // 2. 根据字体高度确定参数和字体数据指针
    switch (font_height) {
        case 12: // 1206字体 (高12, 宽6)
            font_pixel_width = 6;
            bytes_per_col = 2; // (12点高 / 8点每字节) 向上取整
            total_bytes_for_glyph = 12; // 2字节/列 * 6列
            if (char_index_in_font < 95) glyph_data_ptr = &asc2_1206[char_index_in_font][0]; else return;
            break;
        case 16: // 1608字体 (高16, 宽8)
            font_pixel_width = 8;
            bytes_per_col = 2; // (16点高 / 8点每字节)
            total_bytes_for_glyph = 16; // 2字节/列 * 8列
            if (char_index_in_font < 95) glyph_data_ptr = &asc2_1608[char_index_in_font][0]; else return;
            break;
        case 24: // 2412字体 (高24, 宽12)
            font_pixel_width = 12;
            bytes_per_col = 3; // (24点高 / 8点每字节)
            total_bytes_for_glyph = 36; // 3字节/列 * 12列
            if (char_index_in_font < 95) glyph_data_ptr = &asc2_2412[char_index_in_font][0]; else return;
            break;
        default:
            return; // 不支持的字体大小
    }

    // 3. 绘制字符点阵 (逐列式，MSB优先代表列顶部)
    uint8_t current_glyph_byte_idx = 0; // 当前处理到字模数据的第几个字节

    for (uint8_t col = 0; col < font_pixel_width; col++) { // 遍历字符的每一列
        current_col_x = x_start_pos + col;
        if (current_col_x >= TFT_COLUMN_NUMBER) break; // 超出屏幕右边界则停止绘制该字符的后续列

        current_pixel_y = y_start_pos; // 每画新的一列，Y回到起始位置

        for (uint8_t byte_in_current_col = 0; byte_in_current_col < bytes_per_col; byte_in_current_col++) { // 遍历构成当前列的所有字节
            if (current_glyph_byte_idx >= total_bytes_for_glyph) break; // 安全检查，防止越界
            byte_data_from_font = glyph_data_ptr[current_glyph_byte_idx++];

            for (bit_idx_in_byte = 0; bit_idx_in_byte < 8; bit_idx_in_byte++) { // 遍历当前字节的8位
                if ((current_pixel_y - y_start_pos) >= font_height) {
                    break; // 当前列的像素已画完 (因为每列最后一个字节可能包含非字体高度的填充位)
                }
                if (current_pixel_y >= TFT_LINE_NUMBER) {
                    return; // Y坐标超出屏幕则绘制完成或中止
                }

                if (byte_data_from_font & 0x80) { // 检查MSB (最高位)
                    TFT_Draw_Point(current_col_x, current_pixel_y, ECG_TEXT_COLOR);
                } else {
                    if (mode == 0) { // 不透明模式，绘制背景点
                        TFT_Draw_Point(current_col_x, current_pixel_y, ECG_BACKGROUND_COLOR);
                    }
                }
                byte_data_from_font <<= 1; // 左移一位，准备检查下一位
                current_pixel_y++;         // Y坐标向下移动一个像素
            }
        }
    }
}

// 显示数字,高位为0,则不显示 (移植自 LCD_ShowNum)
void TFT_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode) {
    uint8_t t, temp_digit;
    uint8_t enshow = 0;
    uint8_t char_width = size / 2; // 假设字符宽度是高度的一半

    for (t = 0; t < len; t++) {
        temp_digit = (num / TFT_Pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp_digit == 0) {
                if (mode == 0) { // 如果不透明，可以用背景色填充空格区域，或依赖ShowChar的背景填充
                     TFT_ShowChar(x + char_width * t, y, ' ', size, mode);
                }
                // 如果是透明模式，且要显示空格，则需要确保ShowChar能正确处理空格的透明背景
                // 或者在这里直接跳过，不画任何东西
                continue;
            } else {
                enshow = 1;
            }
        }
        TFT_ShowChar(x + char_width * t, y, temp_digit + '0', size, mode);
  }
}

//// 显示字符串 (移植自 LCD_ShowString)
void TFT_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t mode, uint8_t *p) {
    uint16_t x0 = x;
    uint16_t y0 = y;
    uint8_t char_width = size / 2; // 假设字符宽度是高度的一半

    // width 和 height 是相对偏移量，转换为绝对边界
    uint16_t x_end_boundary = x0 + width;
    uint16_t y_end_boundary = y0 + height;


    while ((*p <= '~') && (*p >= ' ')) { // 判断是不是可见ASCII字符
        if (x + char_width > x_end_boundary) { // 需要换行
            x = x0;
            y += size; // 换到下一行
        }
        if (y + size > y_end_boundary) break; // 超出垂直区域

        TFT_ShowChar(x, y, *p, size, mode);
        x += char_width;
        p++;
    }
}



// 确保这些全局变量在某处有定义 (例如 main.c)
// extern uint16_t ecg_waveform_buffer[]; // 实际定义时需要指定大小，如 ecg_waveform_buffer[WAVE_WIDTH]
// extern int current_ecg_x_position;
// extern uint16_t last_ecg_y_value;
// （或者如果这些是ecg_display.c的静态全局变量，则不需要extern声明，
//   但其他文件可能无法直接访问它们，除非通过函数接口）

void ECG_Screen_Init(void) {
    TFT_Clear(ECG_BACKGROUND_COLOR); // 使用ECG背景色清屏

    // 1. 绘制波形显示区域的背景网格 (这部分逻辑保持不变)
    // uint16_t grid_x_end = WAVE_START_X + WAVE_WIDTH - 1;
    // uint16_t grid_y_end = WAVE_START_Y + WAVE_HEIGHT - 1;

    // if (grid_x_end >= TFT_COLUMN_NUMBER) grid_x_end = TFT_COLUMN_NUMBER - 1;
    // if (grid_y_end >= TFT_LINE_NUMBER) grid_y_end = TFT_LINE_NUMBER - 1;

    // int large_grid_step_y = 20;
    // for (int y = WAVE_START_Y; y <= grid_y_end; y += large_grid_step_y) {
    //     if (y < TFT_LINE_NUMBER) {
    //         TFT_Draw_HLine(y, WAVE_START_X, grid_x_end, ECG_GRID_COLOR);
    //     }
    // }
    // int large_grid_step_x = 20;
    // for (int x = WAVE_START_X; x <= grid_x_end; x += large_grid_step_x) {
    //     if (x < TFT_COLUMN_NUMBER) {
    //         TFT_Draw_VLine(x, WAVE_START_Y, grid_y_end, ECG_GRID_COLOR);
    //     }
    // }

    // 2. 绘制静态文本标签 (使用通用的 TFT_ShowString)
    //    在调用 TFT_ShowString 之前，需要设置好全局的 POINT_COLOR 和 BACK_COLOR (如果 mode=0)

    uint8_t label_font_size = 16; // 选择标签的字体大小 (例如16, 对应 asc2_1608)
    uint8_t label_mode = 1;       // 模式1: 透明背景 (推荐用于覆盖在网格上)
                                  // 如果希望背景为 ECG_BACKGROUND_COLOR, 则 mode = 0

    POINT_COLOR = ECG_TEXT_COLOR; // 设置文本的前景色
    if (label_mode == 0) {
        BACK_COLOR = ECG_BACKGROUND_COLOR; // 如果是不透明模式，设置背景色
    }

    // TFT_ShowString 参数: x, y, 绘制区域宽度, 绘制区域高度, 字体大小, 模式, 字符串指针
    // 对于短标签，区域宽度和高度可以根据文本内容和字体大小估算
    // 假设字符宽度是高度的一半 (size/2)

    // 心率标签 "HR:"
    // 区域宽度估算："HR:" 有3个字符，宽度约为 3 * (label_font_size / 2)
    // 区域高度估算：label_font_size
    TFT_ShowString(HR_LABEL_X, HR_LABEL_Y,
                   3 * (label_font_size / 2) + (label_font_size/4), // 区域宽度, 加一点余量
                   label_font_size,             // 区域高度
                   label_font_size, label_mode, (uint8_t*)"HR:");

    // 单位 "bpm"
    // 通常跟在心率数值后面，所以X坐标会依赖HR_VALUE_X和数值的宽度
    // HR_VALUE_X 在 LCD.h 中定义为 (HR_LABEL_X + 3 * CHAR_WIDTH_16PX)
    // 如果 label_font_size 不是16, CHAR_WIDTH_16PX 可能不适用
    // 我们需要根据 label_font_size 计算字符宽度
    uint8_t char_width_for_size = label_font_size / 2;
    uint16_t bpm_label_x = HR_VALUE_X + 3 * char_width_for_size + char_width_for_size / 2; // 假设心率值占3个字符位
    if (bpm_label_x + 3 * char_width_for_size > TFT_COLUMN_NUMBER) { // 防止 "bpm" 超出屏幕
        bpm_label_x = TFT_COLUMN_NUMBER - 3 * char_width_for_size -1; // 粗略调整
    }
    TFT_ShowString(bpm_label_x, HR_LABEL_Y,
                   3 * char_width_for_size + (char_width_for_size/2), label_font_size,
                   label_font_size, label_mode, (uint8_t*)"bpm");


    // 导联名称标签，例如 "II"
    // LEAD_LABEL_X 来自 LCD.h
    TFT_ShowString(LEAD_LABEL_X, LEAD_LABEL_Y,
                   2 * char_width_for_size + (char_width_for_size/2), label_font_size,
                   label_font_size, label_mode, (uint8_t*)"II");


    // // (可选) 其他信息标签，例如ECG增益、走纸速度等
    // // 可以选择不同的字体大小和位置
    // uint8_t info_font_size = 12; // 例如使用小一点的字体
    // uint8_t info_char_width = info_font_size / 2;
    // uint16_t info_label_y = TFT_LINE_NUMBER - info_font_size - 5; // 屏幕底部 (确保不小于0)
    // if (info_label_y > TFT_LINE_NUMBER) info_label_y = TFT_LINE_NUMBER - info_font_size; // 再次检查

    // POINT_COLOR = ECG_TEXT_COLOR; // 确保颜色正确
    // if (label_mode == 0) BACK_COLOR = ECG_BACKGROUND_COLOR;

    // TFT_ShowString(10, info_label_y,
    //                4 * info_char_width, info_font_size, // "x1.0"
    //                info_font_size, label_mode, (uint8_t*)"x1.0");

    // const char* speed_str = "25mm/s";
    // uint16_t speed_str_len = 0;
    // while(speed_str[speed_str_len] != '\0') speed_str_len++;
    // uint16_t speed_label_x = TFT_COLUMN_NUMBER - 5 - (speed_str_len * info_char_width);
    // if (speed_label_x > TFT_COLUMN_NUMBER) speed_label_x = 10; // 防止计算结果为负导致回绕

    // TFT_ShowString(speed_label_x, info_label_y,
    //                speed_str_len * info_char_width + info_char_width, info_font_size, // 走纸速度
    //                info_font_size, label_mode, (uint8_t*)speed_str);


    // 3. 初始化波形缓冲区和相关绘图变量 (这部分逻辑保持不变)
    for(int i=0; i < WAVE_WIDTH; ++i) {
        if (i < (sizeof(ecg_waveform_buffer) / sizeof(ecg_waveform_buffer[0]))) {
            ecg_waveform_buffer[i] = WAVE_BASELINE_Y;
        } else {
            break;
        }
    }
    last_ecg_y_value = WAVE_BASELINE_Y;
    current_ecg_x_position = 0;
}

void ECG_UpdateWaveform(int16_t newDataPoint, int16_t minExpectedData, int16_t maxExpectedData) {
    uint16_t new_y;
    uint16_t x_on_screen_to_update = WAVE_START_X + current_ecg_x_position;

    float scaleFactor = (float)WAVE_HEIGHT / (maxExpectedData - minExpectedData);
    int16_t scaledValue = (int16_t)(newDataPoint * scaleFactor);
    new_y = WAVE_BASELINE_Y - scaledValue;

    if (new_y < WAVE_START_Y) new_y = WAVE_START_Y;
    if (new_y >= WAVE_START_Y + WAVE_HEIGHT) new_y = WAVE_START_Y + WAVE_HEIGHT - 1;

    TFT_Draw_VLine(x_on_screen_to_update, WAVE_START_Y, WAVE_START_Y + WAVE_HEIGHT - 1, ECG_BACKGROUND_COLOR);

    if ((current_ecg_x_position % 10) == 0) {
         TFT_Draw_VLine(x_on_screen_to_update, WAVE_START_Y, WAVE_START_Y + WAVE_HEIGHT - 1, ECG_GRID_COLOR);
    }

    uint16_t actual_prev_x_on_screen = (current_ecg_x_position == 0) ? (WAVE_START_X + WAVE_WIDTH - 1) : (x_on_screen_to_update - 1);
    TFT_DrawLine(actual_prev_x_on_screen, last_ecg_y_value,
                 x_on_screen_to_update, new_y,
                 ECG_WAVEFORM_COLOR);

    ecg_waveform_buffer[current_ecg_x_position] = new_y;
    last_ecg_y_value = new_y;
    current_ecg_x_position = (current_ecg_x_position + 1) % WAVE_WIDTH;
}

void Update_HR_Display(uint16_t hr_value) {
    char hr_string_buffer[6]; // 用于存放HR字符串的缓冲区 (例如 "120", 最大 "65535" + 空字符)
    uint8_t font_size = 16;   // 与 ECG_Screen_Init 中标签使用的字体大小匹配
    uint8_t display_mode = 1; // 透明背景模式 (推荐用于文本，背景已由Fill_Region处理)
    uint8_t char_w = font_size / 2;

    // 1. 定义心率值显示区域的宽度
    //    假设心率值最多显示3位数字 (例如 "250")。
    //    可以根据实际最大心率值调整，例如如果HR可能为1000，则需要4位。
    uint8_t max_hr_digits = 3; // 假设心率最多3位数
    uint16_t hr_value_clear_width = max_hr_digits * char_w; // 清除区域的宽度
                                                        // 可以稍微加一点冗余，例如 (max_hr_digits * char_w) + 2

    // 2. 清除旧的心率值区域
    //    xsta: HR_VALUE_X
    //    ysta: HR_LABEL_Y
    //    xend: HR_VALUE_X + 清除宽度 - 1
    //    yend: HR_LABEL_Y + 字体高度 - 1
    uint16_t clear_x_end = HR_VALUE_X + hr_value_clear_width - 1;
    uint16_t clear_y_end = HR_LABEL_Y + font_size - 1;

    // 调用您的填充函数来用背景色覆盖该区域
    TFT_Fill_Region(HR_VALUE_X, HR_LABEL_Y, clear_x_end, clear_y_end, ECG_BACKGROUND_COLOR);

    // 3. 将新的心率值转换为字符串
    snprintf(hr_string_buffer, sizeof(hr_string_buffer), "%u", hr_value);

    // 4. 设置文本颜色
    POINT_COLOR = ECG_TEXT_COLOR;

    // 5. 显示新的心率字符串
    //    传递给 TFT_ShowString 的 width 参数是其内部用于判断换行的边界，
    //    对于单行短文本，可以设置为清除区域的宽度或者实际文本的宽度。
    //    height 参数也是其内部用于判断是否超出区域的边界。
    TFT_ShowString(HR_VALUE_X, HR_LABEL_Y,
                   hr_value_clear_width, // 字符串绘制区域的最大宽度
                   font_size,            // 字符串绘制区域的高度 (即字体高度)
                   font_size,            // 字体大小参数
                   display_mode,         // 透明模式
                   (uint8_t*)hr_string_buffer);
}

// 在 main.c 或你的 ecg_display.c 中

// 辅助函数：使用 display_char16_16 显示数字 (固定3位数，不足补零或空格)
void ECG_ShowNum_16x16(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t color) {
    char str_num[4]; // 最多3位数 + 空终止符
    uint16_t current_x = x;

    // 将数字转换为字符串，固定长度，不足补前导' '或'0'
    // 这里简单实现，假设len=3
    if (len == 3) {
        if (num < 10) {
            str_num[0] = ' '; // 或者 '0'
            str_num[1] = ' '; // 或者 '0'
            str_num[2] = (num % 10) + '0';
        } else if (num < 100) {
            str_num[0] = ' '; // 或者 '0'
            str_num[1] = (num / 10) + '0';
            str_num[2] = (num % 10) + '0';
        } else {
            str_num[0] = (num / 100) + '0';
            str_num[1] = ((num / 10) % 10) + '0';
            str_num[2] = (num % 10) + '0';
        }
        str_num[3] = '\0';
    } else { // 简单处理其他长度，只显示数字本身（可能需要更复杂的逻辑）
        sprintf(str_num, "%d", num);
    }


    // 清除旧数字区域：用 ECG_BACKGROUND_COLOR 作为前景，白色作为背景（因为 display_char16_16 背景是白色）
    // 绘制三个白色方块来覆盖旧数字
    // 或者，如果 ECG_BACKGROUND_COLOR 本身是白色，那就不需要特别清除，会被覆盖。
    // 假设ECG_BACKGROUND_COLOR不是白色，我们需要先“擦除”
    if (ECG_BACKGROUND_COLOR != WHITE) { // 只有背景不是白色时才需要用白色块擦除
        for (int i = 0; i < len; ++i) {
            // display_char16_16 的背景是固定的白色，我们用白色前景绘制一个字符来模拟填充白色块
            // 或者用 TFT_Fill_Region 更佳
             TFT_Fill_Region(x + i * CHAR_WIDTH_16PX, y,
                             x + (i + 1) * CHAR_WIDTH_16PX - 1, y + CHAR_HEIGHT_16PX - 1,
                             ECG_BACKGROUND_COLOR); // 用背景色填充
        }
    }


    // 显示新数字
    char* p_str = str_num;
//    while (*p_str) {
//        display_char16_16(current_x, y, (unsigned long)color, GetFontIndex(*p_str));
//        current_x += CHAR_WIDTH_16PX;
//        p_str++;
//        if (current_x >= x + len * CHAR_WIDTH_16PX) break; // 确保不超过指定长度
//    }
}


void ECG_UpdateHeartRate(uint16_t hr_value) {
    // 调用新的辅助函数来显示心率值
    ECG_ShowNum_16x16(HR_VALUE_X, HR_VALUE_Y, hr_value, 3, ECG_HR_NUM_COLOR);
}


