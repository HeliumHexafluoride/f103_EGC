/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "fatfs.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD.h"
#include "UI.h"
#include "KEY.h"
#include "rtc.h"
#include "ADS1292R.h"
#include "sd.h"
#include "EGC_dataprocess.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RGB_GROP GPIOA
#define RGB_B GPIO_PIN_1
#define RGB_G GPIO_PIN_2
#define RGB_R GPIO_PIN_3
#define RGB_NO GPIO_PIN_RESET
#define RGB_OFF GPIO_PIN_SET
#define LCD_SHOW_NUM 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void delay_us(uint32_t us);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

char SD_FileName[] = "hello.txt";
uint8_t WriteBuffer[] = "01 write buff to sd \r\n";
uint8_t write_cnt =0;	//鍐橲D鍗℃?★拷???
RTC_TimeTypeDef Time = {0};
RTC_DateTypeDef Date = {0};


uint8_t target_hartrate_show_num=0;
uint8_t target_hart_show_num=0;
float32_t ecg_heart_data_get[FIR_BLOCKSIZE]={0};
float32_t ecg_heart_data_out[FIR_BLOCKSIZE]={0};
float32_t ecg_heart_rate_data_out[FIR_BLOCKSIZE]={0};
int ecg_heart_rate_data[FIR_BLOCKSIZE*PACK_NUM_HR]={0};


uint16_t ecg_heart_data_show_x_now[FIR_BLOCKSIZE]={0};
// uint16_t ecg_heart_data_show_x_last[FIR_BLOCKSIZE]={0};

// float32_t ecg_heart_data_show_y_last[FIR_BLOCKSIZE*LCD_SHOW_NUM]={0};
float32_t ecg_heart_data_show_y_now[FIR_BLOCKSIZE]={0};

ECG_TYPE temp_ecg_data[FIR_BLOCKSIZE];
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_SPI3_Init();
  MX_SPI2_Init();  
  // delay_ms(100);
  MX_FATFS_Init();

  /* USER CODE BEGIN 2 */




  // My_RTC_Init();
  ADS1292_Init(); //初始化ads1292

  EGC_dataprocess_init();
  // RTC_SetSecond_IT_ON();
  delay_ms(100);
  // Get_SDCard_Capacity();
  // WritetoSD(SD_FileName,WriteBuffer,sizeof(WriteBuffer));

  HAL_GPIO_WritePin(RGB_GROP,RGB_R,RGB_OFF);
  HAL_GPIO_WritePin(RGB_GROP,RGB_G,RGB_OFF);
  HAL_GPIO_WritePin(RGB_GROP,RGB_B,RGB_OFF);
  while(Set_ADS1292_Collect(0))//0 正常采集  //1 1mV1Hz内部侧试信号 //2 内部短接噪声测试
  {
      printf("1292寄存器设置失败\r\n");
      delay_ms(1000);
  }

  ADS1292_Recv_Start();




	// unsigned char  *point = &picture_tab[0];
	TFT_BL_1;

  TFT_Init();
  TFT_Init();
  ECG_Screen_Init(); // 绘制ECG屏幕的静态布局




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
// RTC_test--------------------------------------------------------------
    // delay_ms(100);
    // RTC_TimeAndDate_Show();
    // if (HAL_GPIO_ReadPin(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==KEY_ON)
    // {
    //   delay_ms(100);
    //   if (HAL_GPIO_ReadPin(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==KEY_ON){
    //     printf("reset RTC \r\n");
    //     RTC_RESET();
    //     delay_ms(500);
    //   }
    // }
    // if (rccinit_flag)
    // {
    //   HAL_GPIO_WritePin(RGB_GROP,RGB_R,RGB_NO);
    // }
    // else{
    //         HAL_GPIO_WritePin(RGB_GROP,RGB_R,RGB_OFF);
    // }
// RTC_test--------------------------------------------------------------

// FIR-TEST --------------------------------------------------------------

  if(cq_is_full(&ecg_fir_queue)){
    uint16_t HR=0;
    for (u8 i = 0; i < FIR_BLOCKSIZE; i++)
    {
      if(cq_dequeue(&ecg_fir_queue,&temp_ecg_data[i]))
      ecg_heart_data_get[i]=temp_ecg_data[i].ecg_data;
    }
    arm_fir_f32(&S, ecg_heart_data_get,  ecg_heart_data_out,  FIR_BLOCKSIZE);
    compute_integrated_signal(ecg_heart_data_out,ecg_heart_rate_data_out);
    for (u8 i = 0; i < FIR_BLOCKSIZE; i++)
    {
      temp_ecg_data[i].ecg_data=(int)ecg_heart_data_out[i];
      ecg_heart_data_show_x_now[i]=i+target_hart_show_num*250;
      // ecg_heart_data_show_x_last[i]=(ecg_heart_data_show_x_now[i]+750)/1000;
      ecg_heart_data_show_y_now[i]=ecg_heart_data_out[i]*2420.0/16777216.0;

      temp_ecg_data[i].respirat_impedance=(int)ecg_heart_rate_data_out[i]/1000000;
      ecg_heart_rate_data[i+target_hartrate_show_num*250]=temp_ecg_data[i].respirat_impedance;
      HR=hr_count(ecg_heart_rate_data);

      EcgSendByUart_SET_DATA(temp_ecg_data[i]);
    }

    // if(target_hart_show_num==0){
    // draw_ecg_waveform_static(ecg_heart_data_show_y_last+750,
    //                         ecg_heart_data_show_x_last,
    //                         FIR_BLOCKSIZE, // 使用宏定义点数
    //                         BLACK);     // 使用 LCD.h 中定义的颜色
    // }
    // else{
    // draw_ecg_waveform_static(ecg_heart_data_show_y_last-250,
    //                         ecg_heart_data_show_x_last,
    //                         FIR_BLOCKSIZE, // 使用宏定义点数
    //                         BLACK);     // 使用 LCD.h 中定义的颜色
    // }
    // for (u8 i = 0; i < FIR_BLOCKSIZE; i++)
    // ecg_heart_data_show_y_last[i+target_hart_show_num*250]=ecg_heart_data_show_y_now[i];

    target_hartrate_show_num++;
    target_hart_show_num++;

    target_hartrate_show_num%=PACK_NUM_HR;
    target_hart_show_num%=LCD_SHOW_NUM;
    EcgSendByUart_SET_Heart_Rate(HR);

    draw_ecg_waveform_static(ecg_heart_data_show_y_now,
                             ecg_heart_data_show_x_now,
                             FIR_BLOCKSIZE, // 使用宏定义点数
                             ECG_WAVEFORM_COLOR);     // 使用 LCD.h 中定义的颜色
    Update_HR_Display(HR);
  }



// FIR-TEST --------------------------------------------------------------


// TFT-TEST --------------------------------------------------------------
    // HAL_GPIO_WritePin(RGB_GROP,RGB_G,RGB_NO);
    // TFT_Clear(RED);
    // HAL_GPIO_WritePin(RGB_GROP,RGB_G,RGB_OFF);
    // delay_ms(2000);
    // HAL_GPIO_WritePin(RGB_GROP,RGB_G,RGB_NO);
    // TFT_Clear(GREEN);
    // HAL_GPIO_WritePin(RGB_GROP,RGB_G,RGB_OFF);
    // delay_ms(2000);
    // TFT_Clear(BLUE);
    // delay_ms(3000);
    // TFT_Clear(BLACK);
    // Picture_Display(point);
    // delay_ms(3000);
    // display_char16_16(20,160,BLUE,0);
    // display_char16_16(36,160,GREEN,1);
    // display_char16_16(60,160,RED,2);
    // display_char16_16(76,160,BLUE,3);
    // display_char16_16(92,160,GREEN,4);
    // display_char16_16(118,160,BLUE,5);
    // display_char16_16(134,160,RED,6);
    // delay_ms(10000);
// TFT-TEST --------------------------------------------------------------


    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void delay_us(uint32_t us)
{
  uint32_t ticks;
  uint32_t tcnt = 0, told, tnow;
  uint32_t reload = SysTick->LOAD;
  ticks = us * 72;
  told = SysTick->VAL;
  while (1)
  {
    tnow = SysTick->VAL;
    if (tnow < told)
      tcnt += told - tnow;
    else
      tcnt += reload - (tnow - told);
    told = tnow;
    if (tcnt >= ticks)

      break;
  }
}

void LCD_test(){
  
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
