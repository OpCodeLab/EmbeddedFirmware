
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#ifdef MAIN6
IWDG_HandleTypeDef hiwdg;
#define SOF_PATTERN   'A'
void sendACK();

extern void initialise_monitor_handles(void);
UART_HandleTypeDef huart2;
void SendButton(uint8_t button);

int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
    return len;
}

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */
SemaphoreHandle_t   Sem1;
TaskHandle_t        hTask1;
QueueHandle_t       Queue1;
void Task1(void *p);
void Task2(void *p);
void Task3(void *p);
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
uint8_t     RxData[20];
static void MX_IWDG_Init(void);

static void MX_USART2_UART_Init(void);
typedef struct
{
	uint8_t cmd;
	uint16_t config;
	uint32_t data;

}FrameUART;
FrameUART   FrameReceived;
static void PVD_Config(void);
uint8_t SendFrame(FrameUART *frame );
void ExecuteReceivedFrameCallback(FrameUART *frame);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
#ifdef DEBUG
//  initialise_monitor_handles();
  //printf("LAB6------------------- \n");
#endif

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  PVD_Config();
  MX_IWDG_Init();

#ifdef DEBUG
//  initialise_monitor_handles();
  //printf("LAB6------------------- \n");
#endif
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, RxData, 1);

  /*CreateSemaphore */
  Sem1= xSemaphoreCreateBinary();
  /* Create a queue capable of containing 10 unsigned long values. */
  Queue1 = xQueueCreate( 10, sizeof( uint32_t ) );

  /*128 word Stack size / No Pamram / Priority=1, init hTask1->TCB1*/
  xTaskCreate(Task1,"Periodic"  ,128, NULL,2,&hTask1);
  xTaskCreate(Task2,"APeriodic"  ,128,NULL,3,NULL);
  xTaskCreate(Task3,"Processing" ,128,NULL,3,NULL);
 

  /* Create the thread(s) */
  vTaskStartScheduler();
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1);
}

/*Perdidic Task*/
void Task1(void *p) //P1
{
  
  while(1)
  {
   // printf("TAsk 1 is running....\r\n");
    HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_12);
    vTaskDelay(400);
    HAL_IWDG_Refresh(&hiwdg);

  }

}

void Task2(void *p) // P3
{
  TickType_t  FirstTick,SecondTick;
  uint32_t PulseWidth;
  while(1)
  {
   xSemaphoreTake(Sem1,portMAX_DELAY);
   /*Take the FirstTick*/
   FirstTick = xTaskGetTickCount();
   /*While button still forced ...*/
   while( HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==GPIO_PIN_SET ); 
   SecondTick= xTaskGetTickCount();
   PulseWidth=SecondTick-FirstTick;
   xQueueSend(Queue1, (void*)&PulseWidth,100);
   //taskYIELD();
  }
}//end task3




void Task3(void *p) // P4
{
  uint32_t PulseWidth;
  
 while(1)
 {
    // Blocke Task until Queue receive a new data
    xQueueReceive(Queue1, &PulseWidth, portMAX_DELAY );
    if( (PulseWidth<=400) && (PulseWidth>100))
    { 
     // printf("Short Button detected ...\r\n");
      SendButton(0);
    }
    else if (PulseWidth>400) 
    {
    // printf("Long Button detected ...\r\n");
     SendButton(1);
    }
 }
}

void SendButton(uint8_t button)
{
    FrameUART f;
    f.cmd    = 0x55;
    f.config = button;
    f.data= 0x000;

    SendFrame(&f);
}

void sendACK(void)
{
    FrameUART f;
    f.cmd    = 0x20;
    f.config = 0x0000;
    f.data= 0x000;

    SendFrame(&f);
}

void SendPVDFrame(void)
{
    FrameUART f;
    f.cmd    = 0x33;
    f.config = 0x0000;
    f.data= 0x000;

    SendFrame(&f);
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

    HAL_UART_Transmit(&huart2,Txbuffer,8,100);

	return err;


}

// pdFALSE / pdTRUE
/*EXTI0 ISR_Handler*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   BaseType_t xHigherPriority=pdFALSE;
  
   xSemaphoreGiveFromISR(Sem1, &xHigherPriority);
   /*if True force contex switch else wait for next tick*/
   portYIELD_FROM_ISR(xHigherPriority); 
   
}

void HAL_PWR_PVDCallback(void)
{
  /* Toggle LED1 */
  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
  SendPVDFrame();

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
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}
/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

	/*Configure GPIO pin : PA0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PD12 PD13 PD14 PD15 */
	GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}


typedef enum {WAIT_FOR_SOF,
	          WAIT_FOR_CMD,
	         WAIT_FOR_CONFIG,
			 WAIT_FOR_DATA} StateFrame;

StateFrame state=WAIT_FOR_SOF;
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
		sendACK();
		break;
	}

	case COMMAND2:
	{
		/*ACTION1 */
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		sendACK();

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
static void PVD_Config(void)
{
  PWR_PVDTypeDef sConfigPVD;
  /*##-1- Enable Power Clock #################################################*/
  __HAL_RCC_PWR_CLK_ENABLE();

  /*##-2- Configure the NVIC for PVD #########################################*/
  HAL_NVIC_SetPriority(PVD_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(PVD_IRQn);

  /* Configure the PVD Level to 3 and generate an interrupt on rising and falling
     edges(PVD detection level set to 2.5V, refer to the electrical characteristics
     of you device datasheet for more details) */
  sConfigPVD.PVDLevel = PWR_PVDLEVEL_3;
  sConfigPVD.Mode = PWR_PVD_MODE_IT_RISING_FALLING;
  HAL_PWR_ConfigPVD(&sConfigPVD);

  /* Enable the PVD Output */
  HAL_PWR_EnablePVD();
}
/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM9) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while(1)
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
#endif
