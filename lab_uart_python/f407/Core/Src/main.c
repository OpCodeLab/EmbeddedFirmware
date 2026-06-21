/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stdlib.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
typedef struct
{
	uint8_t cmd;
	uint16_t config;
	uint32_t data;

}FrameUART;
void ExecuteReceivedFrameCallback(FrameUART *frame);

FrameUART   FrameReceived;



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t RxData[20];
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
  if (BSP_ACCELERO_Init()!=ACCELERO_OK)
  {
    while(1);
  }
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
   HAL_UART_Receive_IT(&huart2, RxData, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   while (1)
   {
    /*Read From Acceloro*/
    BSP_ACCELERO_GetXYZ(Buffer);
    Xval=Buffer[0];
    Yval=Buffer[1];
  


   }
  /* USER CODE END 3 */
}

uint8_t TxBuffer[14]={"FROM STM32"};

typedef enum {WAIT_FOR_SOF,
	          WAIT_FOR_CMD,
	         WAIT_FOR_CONFIG,
			 WAIT_FOR_DATA} StateFrame;

StateFrame state=WAIT_FOR_SOF;
#define SOF_PATTERN   'A'
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	/*Implement FSM*/
	switch (state)
	{
	 case WAIT_FOR_SOF:
	 {  /*Action*/
		 if (RxData[0]== SOF_PATTERN)
		 {
			 state=WAIT_FOR_CMD;
			HAL_UART_Receive_IT(&huart2, RxData, 1);

		 }
		 else
		 {
				HAL_UART_Receive_IT(&huart2, RxData, 1);

			 /*error*/
		 }
		break;
	 }
	 case WAIT_FOR_CMD:
	 {
		 /*Store Command*/
		 FrameReceived.cmd=RxData[0];
		HAL_UART_Receive_IT(&huart2, RxData, 2);

		 state=WAIT_FOR_CONFIG;
		break;
	 }
	 case WAIT_FOR_CONFIG:
	 {
		 FrameReceived.config=  ( RxData[0] |(RxData[1]<<8));
		 state=WAIT_FOR_DATA;
		 HAL_UART_Receive_IT(&huart2, RxData, 4);

		 break;
	 }
	 case WAIT_FOR_DATA:
	 {
		 FrameReceived.data= ( RxData[0] |(RxData[1]<<8) |(RxData[2]<<16)| (RxData[3]<<24) );
		 /*action*/
		 state=WAIT_FOR_SOF;
		 HAL_UART_Receive_IT(&huart2, RxData, 1);

		 /*Notification*/
		 ExecuteReceivedFrameCallback(&FrameReceived);


		 break;
	 }
	 default:
	 {
		 state=WAIT_FOR_SOF;
	 }

	}

}
#define COMMAND1   0x31
#define COMMAND2   0xD2
#define COMMAND3   0xD3
#define COMMAND4   0xD4


/*Processing*/
void ExecuteReceivedFrameCallback(FrameUART *frame)
{

	switch(frame->cmd)
	{

	case COMMAND1:
	{
		/*ACTION1 */
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		break;
	}

	case COMMAND2:
	{
		/*ACTION1 */
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);

		break;
	}
	case COMMAND3:
	{
		/*ACTION1 */

		break;
	}
	case COMMAND4:
	{
		/*ACTION1 */

		break;
	}


	}


}

uint8_t SendFrame(FrameUART *frame )
{
	uint8_t err=0;

	uint8_t Txbuffer[8];

	Txbuffer[0]=SOF_PATTERN;
	Txbuffer[1]=frame->cmd;
	Txbuffer[2]=(uint8_t)frame->config;
	Txbuffer[3]=(uint8_t)(frame->config>>8);
	Txbuffer[4]=(uint8_t)frame->data;
	Txbuffer[5]=(uint8_t)(frame->data>>8);
	Txbuffer[6]=(uint8_t)(frame->data>>16);
	Txbuffer[7]=(uint8_t)(frame->data>>24);

    HAL_UART_Transmit_IT(&huart2,Txbuffer,8);

	return err;


}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
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
