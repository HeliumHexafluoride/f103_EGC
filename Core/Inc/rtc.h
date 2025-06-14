/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
void My_RTC_Init(void);
void RTC_RESET(void);

extern void Date_write_BKP(RTC_HandleTypeDef *  hrtc,RTC_DateTypeDef * Date);
extern void Date_read_BKP(RTC_HandleTypeDef *  hrtc,RTC_DateTypeDef * Date);
void HAL_RTCEx_RTCEventCallback  ( RTC_HandleTypeDef *  hrtc );


void RTC_TimeAndDate_Show(void);
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
void RTC_SetSecond_IT_ON(void);
extern uint8_t rccinit_flag;
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

