#include "ECG_ANA.h"
#include "lcd.h"
#include "tcp_client.h"

/***********************
ECG_ANA：心电分析，主要为心率计算
***********************/

//采样间隔2ms	每个横轴像素时间间隔：2*ecg_x_ratio（默认ecg_x_ratio = 5）
//测试使用的心电模拟器的心跳周期：760ms（心率 79次/min）每次心跳占像素数：760/(2*ecg_x_ratio)
//测试使用的模拟器 ECG(R波)峰峰值 3.0-3.6mV 
//测试使用的模拟器 R波宽度：75ms
//正常人 QRS波正常处于：60~100ms
//正常人心率：60-120

uint8_t ecg_wave_buf[320];	//用于滑动均值的缓存区
uint32_t ecg_wave_sum;		//滑动均值求和的临时变量
uint32_t ecg_wave_sum_count;//求和计数
uint8_t ecg_wave_avr = 0;	//均值
uint8_t ecg_wave_R_threshold = 0;	//用于检测R波的阈值

extern __IO uint32_t sys_tick;
uint32_t R_tick = 0;			//控制检测R波的周期，防止误判

extern uint8_t heart_rate;		//心率
uint8_t heart_rate_tmp = 0;		//心率临时变量
uint16_t heart_rate_sum = 0;	//心率累加和
uint8_t heart_rate_sum_count = 0;	//心率累加计数
uint8_t heart_rate_err_flag = 0;	//心率错误标志位

//使用过去320个点的滑动均值检测R波实现心率测量
void ecg_analyse(osc_window* win1,uint32_t time,uint8_t time_ratio)
{
	ecg_wave_buf[ecg_wave_sum_count++] = win1->cursor_y;	//记录y值
	if(ecg_wave_sum_count >= 320)
	{
		ecg_wave_sum_count = 0;
	}
	ecg_wave_sum -= ecg_wave_buf[ecg_wave_sum_count];	//去除缓存区尾部值
	ecg_wave_sum += win1->cursor_y;				//累加头部值
	ecg_wave_avr = ecg_wave_sum/320;				//求平均
	if(ecg_wave_avr < win1->loca_y)	//限幅
	{
		ecg_wave_avr = win1->loca_y;
	}
	if(ecg_wave_avr > win1->loca_y+win1->width)	//限幅
	{
		ecg_wave_avr = win1->loca_y+win1->width;
	}
	ecg_wave_R_threshold = ecg_wave_avr-(float)ecg_wave_avr*0.3;//R波阈值 = 1.3*均值
	LCD_DrawPoint(win1->cursor_x,ecg_wave_R_threshold,RED);		//画出R波阈值线
	if(win1->cursor_y < ecg_wave_R_threshold && sys_tick-R_tick>400)	//显示心率
	{
		heart_rate_tmp = 60000/(sys_tick - R_tick);	//计算心率
		if(heart_rate_tmp > 50 && heart_rate_tmp < 120)	//心率在50-120之间才有效
		{
			heart_rate_err_flag = 0;	//数据有效
			heart_rate_sum += heart_rate_tmp;
			heart_rate_sum_count++;
			if(heart_rate_sum_count >= 3)	//求4次心率均值
			{
				heart_rate = heart_rate_sum/heart_rate_sum_count;
				heart_rate_sum = 0;
				heart_rate_sum_count = 0;
			}
		}
		else
		{
			//连续出现3次无效心率数据时，判断为“错误”
			heart_rate_err_flag++;
			if(heart_rate_err_flag >= 3)
			{
				heart_rate = 0;
				heart_rate_sum = 0;
				heart_rate_sum_count = 0;
			}
		}
		R_tick = sys_tick;
	}
	else if(sys_tick-R_tick>20000)	//连续2s未检测到有效心率
	{
		LCD_ShowIntNum(0,140,0,3,GREEN,BLACK,32);	//显示0
	}
}

