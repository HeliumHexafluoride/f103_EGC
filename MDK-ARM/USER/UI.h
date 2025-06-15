#ifndef __UI_H__
#define __UI_H__

#include "LCD.h"
#include "EGC_dataprocess.h"

void draw_ecg_waveform_static(const float32_t y_values[],
                              const uint16_t x_values[],
                              int num_points,
                              uint16_t waveform_color);
															 
															 
															 
															 
															 
															 
															 
#endif //__UI_H__