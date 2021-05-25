/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hw.h"
#include "systemEvent.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

lrwan LRWAN(&huart4) ;
sim7x00 SIM7600E(&huart1) ;
power POWER(&huart3) ;
si7021_t SI7021(&hi2c1) ;
debug SYSTEM(&huart5) ;

int sysStatus = IS_NOT_WARNING ;

int warningEvent = IDLE ;
int updateEvent = IDLE ;

unsigned int MQTTtime = 0 ;
#define MQTT_PERIOD_UPDATE 30

int numTest = 0 ;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void getSystemData() ;
void OS_loop() ;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == UART4){
		LRWAN.IRQhandler() ;
	}
	if(huart->Instance == USART1){
		SIM7600E.IRQhandler() ;
	}
	if(huart->Instance == USART3){
		POWER.IRQhandler() ;
	}
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){
		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

		MQTTtime++ ;
		numTest++ ;

		// Kiểm tra xem đã đến thời hạn để cập nhật dữ liệu MQTT hay chưa ?:
		if ( MQTTtime == MQTT_PERIOD_UPDATE ) {
			updateEvent = SIM_MQTT_UPDATE_DATA ;
			MQTTtime = 0 ;
		}
	}
	HAL_TIM_Base_Start_IT(&htim2) ;
}



void getSystemData() {
	SYSTEM.println("Start measuring Temperature & Humidity : ") ;

	float Temp = 0.0 ;
	float Humi = 0 ;

	// Bắt đầu đọc dữ liệu nhiệt độ và độ ẩm  :
	Temp = (float)SI7021.measureTemperature()/100 ;
	HAL_Delay(100) ;

	Humi = SI7021.measureHumidity();
	HAL_Delay(100) ;

//	// Now get Power Manager data :
	POWER.getPeriodData() ;

	char CharTemp[5] ;
	char CharHumi[5] ;
	char charStatus[1] ;
	sprintf(CharTemp, "%.2f", Temp);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc
	sprintf(CharHumi, "%.0f", Humi);	// Cach chuyen float thanh char[x], Neu muon thanh char* thi ep kieu la duoc
	sprintf(charStatus, "%d", sysStatus);



#ifdef SENSOR_DEBUG
	SYSTEM.println("Temp : ") ;
	SYSTEM.print(CharTemp) ;
	SYSTEM.println("Humi : ") ;
	SYSTEM.print(CharHumi) ;
	SYSTEM.println("Currently status : ") ;
	SYSTEM.print(charStatus) ;

	SYSTEM.println("Power data : ") ;
	SYSTEM.print(POWER.powData) ;
//	SYSTEM.println(POWER.U_out) ;
//	SYSTEM.println(POWER.spower) ;
//	SYSTEM.println(POWER.I_pv) ;
//	SYSTEM.println(POWER.I_load) ;

#endif

	// Đưa dữ liệu vào bộ đệm
	LRWAN.updateData( Temp, Humi, POWER.powData, sysStatus ) ;
//	LRWAN.updateData( Temp, Humi, "00.00", "00.00", "00.00", "00.00", sysStatus ) ;


}



void OS_loop(){

	if (  SIM7600E.rxDone_FLAG == true  ){
		SIM7600E.checkingAndProcess() ;

		SIM7600E.rxDone_FLAG = false ;
		SIM7600E.dataCount = 0 ;
		memset ( SIM7600E.rxData, 0 , (size_t)sizeof(SIM7600E.rxData)/sizeof(char) ) ;
	}

	if (  LRWAN.rxDone_FLAG == true  ){
		LRWAN.checkingAndProcess() ;

		LRWAN.rxDone_FLAG = false ;
		LRWAN.dataCount = 0 ;
		memset ( LRWAN.rxData, 0 , (size_t)sizeof(LRWAN.rxData)/sizeof(char) ) ;
	}


	switch (warningEvent) {
		case WARNING_LEVEL_1 :
			if ( sysStatus != IS_NOT_WARNING )
				SIM7600E.warningStop() ; // stop warning before new warning
			SIM7600E.warningStart(WARNING_LEVEL_1, 100) ;
			warningEvent = IDLE ;
			sysStatus = IS_WARNING_LEVEL_1 ;
			break;
		case WARNING_LEVEL_2 :
			if ( sysStatus != IS_NOT_WARNING )
				SIM7600E.warningStop() ;
			SIM7600E.warningStart(WARNING_LEVEL_2, 100) ;
			warningEvent = IDLE ;
			sysStatus = IS_WARNING_LEVEL_2 ;
			break;
		case WARNING_LEVEL_3 :
			if ( sysStatus != IS_NOT_WARNING )
				SIM7600E.warningStop() ;
			SIM7600E.warningStart(WARNING_LEVEL_3, 100) ;
			warningEvent = IDLE ;
			sysStatus = IS_WARNING_LEVEL_3 ;
			break;
		case WARNING_LEVEL_4 :
			if ( sysStatus != IS_NOT_WARNING )
				SIM7600E.warningStop() ;
			SIM7600E.warningStart(WARNING_LEVEL_4, 100) ;
			warningEvent = IDLE ;
			sysStatus = IS_WARNING_LEVEL_4 ;
			break;
		case WARNING_LEVEL_5 :
			if ( sysStatus != IS_NOT_WARNING )
				SIM7600E.warningStop() ;
			SIM7600E.warningStart(WARNING_LEVEL_5, 100) ;
			warningEvent = IDLE ;
			sysStatus = IS_WARNING_LEVEL_5 ;
			break;
		case STOP_WARNING :
			SIM7600E.warningStop() ;
			warningEvent = IDLE ;
			sysStatus = IS_NOT_WARNING ;
			break;
		case IDLE :
			// do nothing
			break ;
		default:
			SYSTEM.println("UNKNOWN WARNING EVENT !\n") ;
			break;
	}

	switch (updateEvent) {
		case SIM_MQTT_UPDATE_DATA :
			SYSTEM.println("SIM_MQTT_UPDATE_DATA") ;

			updateEvent = IDLE ;
			break;
		case LORAWAN_UPDATE_DATA :
			SYSTEM.println("LORAWAN_UPDATE_DATA") ;

			// Lấy dữ liệu hệ thống
			getSystemData() ;

			// Bắt đầu gửi lên server
			LRWAN.send() ;

			updateEvent = IDLE ;
			break;
		case IDLE :
			// do nothing
			break ;
		default:
			SYSTEM.println("UNKNOWN UPDATE EVENT !\n") ;
			break;
	}

	// enter sleep mode here
	if ( ( warningEvent == IDLE ) && (updateEvent == IDLE) ){
		// Enter sleep mode & waiting for interrupt wake-up

	}

	// Reload counter for Watchdog timer :
	IWDG->KR = 0xAAAA;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_I2C1_Init();
  MX_IWDG_Init();
  MX_TIM2_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  SYSTEM.println("\nSTARTING ... ") ;
  SYSTEM.println("\nPlease waiting for First initializing and setup... ") ;
  SYSTEM.println("\n( Make sure pressed POWER KEY button on SIM7600E module then the led is blinking )") ;
  // IO first setup
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2  | GPIO_PIN_4  | GPIO_PIN_5 | GPIO_PIN_6  |
		  	  	  	  	   GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9 | GPIO_PIN_10 |
						   GPIO_PIN_11, (GPIO_PinState) 0) ;

  // Init temperature sensor
  SI7021.init() ;

  SIM7600E.init() ;

  LRWAN.init() ;

  POWER.init() ;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  SYSTEM.println("\nComplete !") ;

  SYSTEM.println("\nNOW SYSTEM IS WORKING : ") ;

  HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  OS_loop() ;
//	  if ( numTest >= 5 ) {
//		  SYSTEM.println("Sending data to Power ") ;
//		  HAL_UART_Transmit(&huart3, (uint8_t*)"RPD\n", sizeof("RPD\n")/sizeof(uint8_t), 2000) ;
//		  numTest = 0 ;
//	  }
//	  IWDG->KR = 0xAAAA;

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 9;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_HSI;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_HSI;
  PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_HSI;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
