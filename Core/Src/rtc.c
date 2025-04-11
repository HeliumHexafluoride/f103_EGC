/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    rtc.c
 * @brief   This file provides code for the configuration
 *          of the RTC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
uint16_t Rtctmp=0;
uint8_t rccinit_flag;
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x12;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_THURSDAY;
  DateToUpdate.Month = RTC_MONTH_APRIL;
  DateToUpdate.Date = 0x10;
  DateToUpdate.Year = 0x25;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */


  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_IRQn);
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void Date_write_BKP(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *Date)
{ // 将日期保存在备份
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE(); 
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR2, (uint16_t)Date->Date);
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR3, (uint16_t)Date->WeekDay);
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR4, (uint16_t)Date->Month);
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR5, (uint16_t)Date->Year);
}

void Date_read_BKP(RTC_HandleTypeDef *hrtc,RTC_DateTypeDef *gDate)
{ // 读取备份日期
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE(); 
  HAL_PWR_EnableBkUpAccess();
  gDate->Year = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR5);
  gDate->Month = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR4);
  gDate->WeekDay = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR3);
  gDate->Date = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR2);

}

void My_RTC_Init(void)
{
  RTC_TimeTypeDef sTime={0};
  RTC_DateTypeDef DateToUpdate ={0};
  rccinit_flag=0;
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_RTC_SECOND_ENABLE_IT(&hrtc,RTC_IT_SEC);
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE(); 
  HAL_PWR_EnableBkUpAccess();  
  HAL_RTC_WaitForSynchro(&hrtc);  
  // 第一次重启，初始化时钟和BKP
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1 )!= 0x5051)
  {
    rccinit_flag=1;
    printf("reinit RTC\r\n");
    sTime.Hours = 0x12;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }
    DateToUpdate.WeekDay = RTC_WEEKDAY_THURSDAY;
    DateToUpdate.Month = RTC_MONTH_APRIL;
    DateToUpdate.Date = 0x10;
    DateToUpdate.Year = 0x25;

    if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }
    Date_write_BKP(&hrtc, &DateToUpdate);
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE(); 
    HAL_PWR_EnableBkUpAccess();  
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5051);    
  }
  // 否则从BKP更新日期
  else
  {
    printf("get time from BKP\r\n");
    // HAL_RTC_MspInit(&hrtc);
    Date_read_BKP(&hrtc,&DateToUpdate);
    if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
    {
    Error_Handler();
    }
  }

  // HAL_RTCEx_SetSecond_IT(&hrtc);
}



void RTC_RESET(void){
  RTC_DateTypeDef DateToUpdate ={0};
    MX_RTC_Init();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5051);
    DateToUpdate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
    DateToUpdate.Month = RTC_MONTH_MAY;
    DateToUpdate.Date = 0x09;
    DateToUpdate.Year = 0x24;
    Date_write_BKP(&hrtc, &DateToUpdate);    
    if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }

}





void RTC_TimeAndDate_Show(void)
{


  RTC_TimeTypeDef RTC_TimeStructure;
  RTC_DateTypeDef RTC_DateStructure;
  // 获取日历
  HAL_RTC_GetTime(&hrtc, &RTC_TimeStructure, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &RTC_DateStructure, RTC_FORMAT_BIN);

  // 每秒打印�????????????�????????????
  if (Rtctmp != RTC_TimeStructure.Seconds)
  {

    // 打印日期
    printf("The Date :  Y:20%0.2d - M:%0.2d - D:%0.2d - W:%0.2d\r\n",
            RTC_DateStructure.Year,
            RTC_DateStructure.Month,
            RTC_DateStructure.Date,
            RTC_DateStructure.WeekDay);

    // 打印时间
    printf("The Time :  %0.2d:%0.2d:%0.2d \r\n\r\n",
            RTC_TimeStructure.Hours,
            RTC_TimeStructure.Minutes,
            RTC_TimeStructure.Seconds);
  }
  Rtctmp = RTC_TimeStructure.Seconds;
}

RTC_TimeTypeDef RTC_Time_Get(void)
{

  RTC_TimeTypeDef RTC_TimeStructure;
  RTC_DateTypeDef RTC_DateStructure;
  HAL_RTC_GetTime(&hrtc, &RTC_TimeStructure, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &RTC_DateStructure, RTC_FORMAT_BIN);

  return RTC_TimeStructure;
}

RTC_DateTypeDef RTC_Date_Get(void)
{

  RTC_DateTypeDef RTC_DateStructure;
  HAL_RTC_GetDate(&hrtc, &RTC_DateStructure, RTC_FORMAT_BIN);

  return RTC_DateStructure;
}

void HAL_RTCEx_RTCEventCallback ( RTC_HandleTypeDef *  hrtc ) 
{
  //秒中断更新时�???????
  RTC_DateTypeDef sDate,gDate;
  printf("update time\r\n");  
	Date_read_BKP(hrtc,&gDate);
	HAL_RTC_GetDate(hrtc,&sDate,RTC_FORMAT_BCD);
	//如果日期有变化就保存当前日期到备份域BKP为下次掉电上电后计算日期提供初始化数�???????
	if(gDate.Date!=sDate.Date)
	{
  printf("update date\r\n");
	gDate.Date=sDate.Date;
	gDate.WeekDay=sDate.WeekDay;
	gDate.Month=sDate.Month;
	gDate.Year=sDate.Year;
	Date_write_BKP(hrtc,&gDate);
	}
}


/* USER CODE END 1 */
