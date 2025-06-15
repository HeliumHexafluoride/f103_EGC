// ������������������ UI��ص� .c �ļ��� main.c ��
// ȷ�������� LCD.h �Ի�ȡ��Ļ��������ɫ����
// #include "LCD.h"

/**
 * @brief ��ԭʼECG���ݣ�����X�����꣩��������ӳ�䵽��Ļ��������Ȼ�󻭳����ݵ㡣
 *
 * @param original_ecg_values   ָ��洢ECG����ֵ������ (���� original_data[0])
 * @param original_x_coords     ָ��洢ԭʼX����������� (���� original_data[1])
 *                               ���ԭʼX������ʽ��0��num_original_points-1����˲�������ΪNULL��
 *                               �����ڲ�������0��num_original_points-1��X���ꡣ
 * @param num_original_points   ԭʼ���ݵ������ (���� 1000)
 * @param screen_wave_width     ��Ļ�ϲ�������Ŀ�� (ͨ���� WAVE_WIDTH, ���� 320)
 * @param clear_background      �Ƿ��ڻ���ǰ����������� (true/false)
 */
 
#include "UI.h"
#include "LCD.h"
#include "EGC_dataprocess.h"


// ������������������ UI��ص� .c �ļ��� main.c ��
// ȷ�������� LCD.h �Ի�ȡ��Ļ��������ɫ����
// #include "LCD.h"
// �Լ� stdbool.h (��� bool ����δ����)
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

// #define CLEAR_STRIP_WIDTH 1 // ÿ�����1�������Ի��ƽ��Ч�������Ը�����Ҫ����Ϊ5��10�ȡ�

// void draw_ecg_waveform_static(const float32_t y_values[],
//                               const uint16_t x_values[],
//                               int num_points,
//                               uint16_t waveform_color) {

//     // 1. ���㲨��ʵ�ʿɻ���/���������߽磬��������Ļ�ü�
//     uint16_t wave_draw_area_x_start = WAVE_START_X;
//     uint16_t wave_draw_area_y_start = WAVE_START_Y;

//     uint16_t wave_draw_area_x_end = WAVE_START_X + WAVE_WIDTH - 1;
//     if (wave_draw_area_x_end >= TFT_COLUMN_NUMBER) { // TFT_COLUMN_NUMBER ����Ļ���߼����
//         wave_draw_area_x_end = TFT_COLUMN_NUMBER - 1;
//     }

//     uint16_t wave_draw_area_y_end = WAVE_START_Y + WAVE_HEIGHT - 1;
//     if (wave_draw_area_y_end >= TFT_LINE_NUMBER) {   // TFT_LINE_NUMBER ����Ļ���߼��߶�
//         wave_draw_area_y_end = TFT_LINE_NUMBER - 1;
//     }

//     // ȷ��������Ļ���������Ч (��ʼ���겻���ڽ�������)
//     if (wave_draw_area_x_start > wave_draw_area_x_end || wave_draw_area_y_start > wave_draw_area_y_end) {
//         return; // ���������Ч����ִ���κβ���
//     }

//     // 2. �����ң��ñ���ɫ���ǣ��������������
//     for (uint16_t x_clear_strip_start = wave_draw_area_x_start;
//          x_clear_strip_start <= wave_draw_area_x_end;
//          x_clear_strip_start += CLEAR_STRIP_WIDTH) {

//         uint16_t current_clear_strip_x_end = x_clear_strip_start + CLEAR_STRIP_WIDTH - 1;
//         if (current_clear_strip_x_end > wave_draw_area_x_end) {
//             current_clear_strip_x_end = wave_draw_area_x_end; //ȷ�������������ұ߽�
//         }

//         // ��䵱ǰ��ֱ����
//         TFT_Fill_Region(x_clear_strip_start,
//                         wave_draw_area_y_start,
//                         current_clear_strip_x_end,
//                         wave_draw_area_y_end,
//                         ECG_BACKGROUND_COLOR);

//         // ��ѡ: �����Ҫ�����Ե��Ӿ�����Ч���������ڴ˴�����ǳ�С����ʱ��
//         // HAL_Delay(1); // �������Զ������ʱ������ע�⣺�������Ӱ�����ܡ�
//     }
//     // ���ˣ��������λ��������ѱ�����ɫ���ǡ�

//     // 3. �����µĲ�������
//     float input_x_range = INPUT_X_DATA_MAX - INPUT_X_DATA_MIN;
//     float input_y_range = INPUT_Y_DATA_MAX - INPUT_Y_DATA_MIN;

//     // ���������
//     if (input_x_range == 0.0f) input_x_range = 1.0f;
//     if (input_y_range == 0.0f) input_y_range = 1.0f;
//     float last_input_x = 0;
//     float last_input_y = 0;
//     for (int i = 0; i < num_points; ++i) {
//         float current_input_x = (float)x_values[i];
//         float current_input_y = y_values[i];

//         // ������X�����һ���� [0, 1] ��Χ
//         float normalized_x = (current_input_x - INPUT_X_DATA_MIN) / input_x_range;
//         // ����һ����X����ӳ�䵽��Ļ���������X����
//         // ע�⣺�������ǻ���ԭʼ�趨�� WAVE_WIDTH�������ţ�Ȼ����� WAVE_START_X ƫ��
//         uint16_t screen_x = WAVE_START_X + (uint16_t)(normalized_x * (WAVE_WIDTH - 1.0f) + 0.5f);

//         // ������Y�����һ���� [0, 1] ��Χ
//         float normalized_y = (current_input_y - INPUT_Y_DATA_MIN) / input_y_range;
//         // ����һ����Y����ӳ�䵽��Ļ���������Y���� (ע��Y�ᷴת)
//         // ����ԭʼ�趨�� WAVE_HEIGHT �������ţ�Ȼ����� WAVE_START_Y ƫ��
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
    float64_t a = 0.0,b = 0.0;         // ����ʽ: y=ax+b
    uint16_t sx = MIN(x1, x2);           // sx �� ex ��Ҫ�ɶ�ʹ�� y��x���ܻ���
    uint16_t sy = MIN(y1, y2);
    uint16_t ex = MAX(x1, x2);
    uint16_t ey = MAX(y1, y2);
    if (x1 == x2)
    {
        //��ֱ��
        for(uint16_t i = sy;i < ey;i++)
            TFT_Draw_Point(x1, i, rgb);
    }
    else if (y1 == y2)
    {
        //ˮƽ��
        for(uint16_t i = sx;i < ex;i++)
            TFT_Draw_Point(i, y1, rgb);
    }
    else if((ex-sx) < (ey-sy))      //ʹ�������Сֵ�ж�ȷ�� ��ֵ��Ϊ����
    {
        //ʹ��y������ɨ��
        a = (x1 - x2) * 1.0/(y1 - y2);//y�����б��Ϊ ��x/��y
        b = x1 - a * y1;
        for(uint16_t i = sy;i< ey;i++)
            TFT_Draw_Point((uint16_t)(a*i+b), i, rgb);
    }
    else
    {
        //ʹ��x������ɨ��
        a = (y1 - y2) * 1.0/ (x1 - x2);//y�����б��Ϊ ��x/��y
        b = y1 - a * x1;
        for(uint16_t i = sy;i< ey;i++)
            TFT_Draw_Point(i,(uint16_t)(a*i+b), rgb);
    }
}
