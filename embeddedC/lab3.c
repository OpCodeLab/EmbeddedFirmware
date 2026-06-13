/**
  ******************************************************************************
  * @file           : lab3.c
  * @brief          : structure & union &bitfield
  ******************************************************************************
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"


#ifdef LAB3
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);


union Number
{
    int x;
    float y;
    char z;
};


//#pragma pack(1)
typedef struct
{
  uint8_t a;   // 1 byte
 // __attribute__ ((aligned (8)))  uint16_t  b;  // 4 bytes
  uint32_t b;
  uint8_t c;
} temp1;


typedef struct
{
  uint8_t a;   // 1 byte
///  __attribute__ ((aligned (8)))  uint32_t  b;  // 4 bytes
  //uint32_t b; __attribute__ ((aligned (8)))
  uint16_t b;
  uint8_t c;   // 1 byte
}   __attribute__((packed)) temp2;

temp1 st1;
temp2 st2;

uint8_t tab[]= {1,2,3,4,5,7} ;
//uint8_t tab[] __attribute__ ((aligned (8))) ={1,2,3,4,5,7} ;

uint8_t a_offset;
uint8_t b_offset;
uint8_t c_offset;

uint8_t a2_offset;
uint8_t b2_offset;
uint8_t c2_offset;

uint8_t a_align;
uint8_t b_align;
uint8_t c_align;

uint8_t a2_align;
uint8_t b2_align;
uint8_t c2_align;


typedef struct
{
  uint16_t Bit0:1;
  uint16_t Bit1:1;
  uint16_t Bit2:1;
  uint16_t Bit3:1;
  uint16_t Bit4:1;
  uint16_t Bit5:1;
  uint16_t Bit6:1;
  uint16_t Bit7:1;
  uint16_t Bit8:1;
  uint16_t Bit9:1;
  uint16_t Bit10:1;
  uint16_t Bit11:1;
  uint16_t Bit12:1;
  uint16_t Bit13:1;
  uint16_t Bit14:1;
  uint16_t Bit15:1;
}IOField ;

IOField  *GPIOD_IO;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  union Number num;


  GPIOD_IO=(IOField*)(&GPIOD->ODR);

  num.x= 15;
  num.y= 3.34;
  num.z= 'A';

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();



  st1.a =5 ;
  /*------------TEMP1 */
  /*offset of*/
  a_offset=__builtin_offsetof(temp1,a);
  b_offset=__builtin_offsetof(temp1,b);
  c_offset=__builtin_offsetof(temp1,c);

  /*Align of */
  a_align=__alignof__(st1.a);
  b_align=__alignof__(st1.b);
  c_align=__alignof__(st1.c);



  /*------------TEMP1 */
  /*offset of*/
  a2_offset=__builtin_offsetof(temp2,a);
  b2_offset=__builtin_offsetof(temp2,b);
  c2_offset=__builtin_offsetof(temp2,c);

  /*Align of */
  a2_align=__alignof__(st2.a);
  b2_align=__alignof__(st2.b);
  c2_align=__alignof__(st2.c);






  st1.b =50 ;

  st2.a=9;




  /* Infinite loop */
  while (1)
  {

    GPIOD_IO->Bit12^= 1;
      HAL_Delay(200);


  }
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{



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
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

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
#endif
