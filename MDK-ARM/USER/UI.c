// 假设这个函数放在你的 UI相关的 .c 文件或 main.c 中
// 确保包含了 LCD.h 以获取屏幕参数和颜色定义
// #include "LCD.h"

/**
 * @brief 将原始ECG数据（包含X轴坐标）降采样并映射到屏幕波形区域，然后画出数据点。
 *
 * @param original_ecg_values   指向存储ECG数据值的数组 (例如 original_data[0])
 * @param original_x_coords     指向存储原始X轴坐标的数组 (例如 original_data[1])
 *                               如果原始X轴是隐式的0到num_original_points-1，则此参数可以为NULL，
 *                               函数内部会生成0到num_original_points-1的X坐标。
 * @param num_original_points   原始数据点的数量 (例如 1000)
 * @param screen_wave_width     屏幕上波形区域的宽度 (通常是 WAVE_WIDTH, 例如 320)
 * @param clear_background      是否在绘制前清除波形区域 (true/false)
 */
 
#include "UI.h"
#include "LCD.h"
#include "EGC_dataprocess.h"


// 假设这个函数放在你的 UI相关的 .c 文件或 main.c 中
// 确保包含了 LCD.h 以获取屏幕参数和颜色定义
// #include "LCD.h"
// 以及 stdbool.h (如果 bool 类型未定义)
// #include <stdbool.h>
// --- Input data range constants ---
// X-axis input data range (e.g., sample index or time)
const float INPUT_X_DATA_MIN = 0.0f;
const float INPUT_X_DATA_MAX = 250.0f; // Matching your original_x_coords max value

// Y-axis input data range (using defined ECG value limits)
const float INPUT_Y_DATA_MIN = MIN_ECG_DATA_VALUE; // -10.0f
const float INPUT_Y_DATA_MAX = MAX_ECG_DATA_VALUE; //  10.0f
void lcd_set_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t rgb);

void draw_ecg_waveform_static(const float32_t y_values[],
                              const uint16_t x_values[],
                              int num_points,
                              uint16_t waveform_color) {

    float input_x_range = INPUT_X_DATA_MAX - INPUT_X_DATA_MIN;
    float input_y_range = INPUT_Y_DATA_MAX - INPUT_Y_DATA_MIN;

    // Avoid division by zero if range is zero (e.g., if all points are the same)
    if (input_x_range == 0.0f) input_x_range = 1.0f;
    if (input_y_range == 0.0f) input_y_range = 1.0f;

    uint16_t last_input_x = 0;
    uint16_t last_input_y = 0;
    
    for (int i = 0; i < num_points; ++i) {
        float current_input_x = (float)x_values[i];
        float current_input_y = y_values[i];

        // 1. Normalize X coordinate to [0, 1]
        float normalized_x = (current_input_x - INPUT_X_DATA_MIN) / input_x_range;

        // 2. Map normalized X to screen wave area X coordinates
        //    Screen X: [WAVE_START_X, WAVE_START_X + WAVE_WIDTH - 1]
        uint16_t screen_x = WAVE_START_X + (uint16_t)(normalized_x * (WAVE_WIDTH - 1.0f) + 0.5f);

        // 3. Normalize Y coordinate to [0, 1]
        float normalized_y = (current_input_y - INPUT_Y_DATA_MIN) / input_y_range;

        // 4. Map normalized Y to screen wave area Y coordinates
        //    Screen Y: [WAVE_START_Y, WAVE_START_Y + WAVE_HEIGHT - 1]
        //    Input Y_MIN maps to bottom of wave area (WAVE_START_Y + WAVE_HEIGHT - 1)
        //    Input Y_MAX maps to top of wave area (WAVE_START_Y)
        uint16_t screen_y = WAVE_START_Y + (uint16_t)((1.0f - normalized_y) * (WAVE_HEIGHT - 1.0f) + 0.5f);
        if (last_input_x==0){
        last_input_x=screen_x;
        last_input_y=screen_y;
        }

        // TFT_Draw_Point itself has boundary checks against TFT_COLUMN_NUMBER and TFT_LINE_NUMBER.
        // If WAVE_START_X + WAVE_WIDTH > TFT_COLUMN_NUMBER or
        // WAVE_START_Y + WAVE_HEIGHT > TFT_LINE_NUMBER, points outside physical screen will be clipped.
        TFT_Fill_Region(last_input_x,WAVE_START_Y,last_input_x+5,WAVE_BOTTOM_Y,BLACK);
        // lcd_set_line(last_input_x,last_input_y,screen_x, screen_y, waveform_color);
        TFT_DrawLine(last_input_x,last_input_y,screen_x, screen_y, waveform_color);
        last_input_x=screen_x;
        last_input_y=screen_y;
    }
}

// #define CLEAR_STRIP_WIDTH 1 // 每次清除1列像素以获得平滑效果。可以根据需要调整为5或10等。

// void draw_ecg_waveform_static(const float32_t y_values[],
//                               const uint16_t x_values[],
//                               int num_points,
//                               uint16_t waveform_color) {

//     // 1. 计算波形实际可绘制/清除的区域边界，并进行屏幕裁剪
//     uint16_t wave_draw_area_x_start = WAVE_START_X;
//     uint16_t wave_draw_area_y_start = WAVE_START_Y;

//     uint16_t wave_draw_area_x_end = WAVE_START_X + WAVE_WIDTH - 1;
//     if (wave_draw_area_x_end >= TFT_COLUMN_NUMBER) { // TFT_COLUMN_NUMBER 是屏幕的逻辑宽度
//         wave_draw_area_x_end = TFT_COLUMN_NUMBER - 1;
//     }

//     uint16_t wave_draw_area_y_end = WAVE_START_Y + WAVE_HEIGHT - 1;
//     if (wave_draw_area_y_end >= TFT_LINE_NUMBER) {   // TFT_LINE_NUMBER 是屏幕的逻辑高度
//         wave_draw_area_y_end = TFT_LINE_NUMBER - 1;
//     }

//     // 确保计算出的绘制区域有效 (起始坐标不大于结束坐标)
//     if (wave_draw_area_x_start > wave_draw_area_x_end || wave_draw_area_y_start > wave_draw_area_y_end) {
//         return; // 如果区域无效，则不执行任何操作
//     }

//     // 2. 从左到右，用背景色覆盖（清除）波形区域
//     for (uint16_t x_clear_strip_start = wave_draw_area_x_start;
//          x_clear_strip_start <= wave_draw_area_x_end;
//          x_clear_strip_start += CLEAR_STRIP_WIDTH) {

//         uint16_t current_clear_strip_x_end = x_clear_strip_start + CLEAR_STRIP_WIDTH - 1;
//         if (current_clear_strip_x_end > wave_draw_area_x_end) {
//             current_clear_strip_x_end = wave_draw_area_x_end; //确保不超过区域右边界
//         }

//         // 填充当前垂直条带
//         TFT_Fill_Region(x_clear_strip_start,
//                         wave_draw_area_y_start,
//                         current_clear_strip_x_end,
//                         wave_draw_area_y_end,
//                         ECG_BACKGROUND_COLOR);

//         // 可选: 如果需要更明显的视觉擦除效果，可以在此处加入非常小的延时。
//         // HAL_Delay(1); // 或其他自定义的延时函数。注意：这会显著影响性能。
//     }
//     // 至此，整个波形绘制区域已被背景色覆盖。

//     // 3. 绘制新的波形数据
//     float input_x_range = INPUT_X_DATA_MAX - INPUT_X_DATA_MIN;
//     float input_y_range = INPUT_Y_DATA_MAX - INPUT_Y_DATA_MIN;

//     // 避免除以零
//     if (input_x_range == 0.0f) input_x_range = 1.0f;
//     if (input_y_range == 0.0f) input_y_range = 1.0f;
//     float last_input_x = 0;
//     float last_input_y = 0;
//     for (int i = 0; i < num_points; ++i) {
//         float current_input_x = (float)x_values[i];
//         float current_input_y = y_values[i];

//         // 将输入X坐标归一化到 [0, 1] 范围
//         float normalized_x = (current_input_x - INPUT_X_DATA_MIN) / input_x_range;
//         // 将归一化的X坐标映射到屏幕波形区域的X坐标
//         // 注意：这里我们基于原始设定的 WAVE_WIDTH进行缩放，然后加上 WAVE_START_X 偏移
//         uint16_t screen_x = WAVE_START_X + (uint16_t)(normalized_x * (WAVE_WIDTH - 1.0f) + 0.5f);

//         // 将输入Y坐标归一化到 [0, 1] 范围
//         float normalized_y = (current_input_y - INPUT_Y_DATA_MIN) / input_y_range;
//         // 将归一化的Y坐标映射到屏幕波形区域的Y坐标 (注意Y轴反转)
//         // 基于原始设定的 WAVE_HEIGHT 进行缩放，然后加上 WAVE_START_Y 偏移
//         uint16_t screen_y = WAVE_START_Y + (uint16_t)((1.0f - normalized_y) * (WAVE_HEIGHT - 1.0f) + 0.5f);
//         if (last_input_x==0){
//         last_input_x=screen_x;
//         last_input_y=screen_y;
//         }
//         lcd_set_line(last_input_x,last_input_y,screen_x, screen_y, waveform_color);
//         last_input_x=screen_x;
//         last_input_y=screen_y;
//     }
// }

void lcd_set_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t rgb)
{
    float64_t a = 0.0,b = 0.0;         // 解析式: y=ax+b
    uint16_t sx = MIN(x1, x2);           // sx 与 ex 需要成对使用 y与x不能混用
    uint16_t sy = MIN(y1, y2);
    uint16_t ex = MAX(x1, x2);
    uint16_t ey = MAX(y1, y2);
    if (x1 == x2)
    {
        //垂直线
        for(uint16_t i = sy;i < ey;i++)
            TFT_Draw_Point(x1, i, rgb);
    }
    else if (y1 == y2)
    {
        //水平线
        for(uint16_t i = sx;i < ex;i++)
            TFT_Draw_Point(i, y1, rgb);
    }
    else if((ex-sx) < (ey-sy))      //使用最大最小值判断确保 Δ值不为负数
    {
        //使用y方向做扫描
        a = (x1 - x2) * 1.0/(y1 - y2);//y方向的斜率为 Δx/Δy
        b = x1 - a * y1;
        for(uint16_t i = sy;i< ey;i++)
            TFT_Draw_Point((uint16_t)(a*i+b), i, rgb);
    }
    else
    {
        //使用x方向做扫描
        a = (y1 - y2) * 1.0/ (x1 - x2);//y方向的斜率为 Δx/Δy
        b = y1 - a * x1;
        for(uint16_t i = sy;i< ey;i++)
            TFT_Draw_Point(i,(uint16_t)(a*i+b), rgb);
    }
}
