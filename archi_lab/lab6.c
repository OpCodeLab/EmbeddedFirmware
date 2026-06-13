/**
  ******************************************************************************
  * @file           : lab6.c
  * @brief          : change clock lowpower-high power
  *******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#ifdef LAB6
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config_HP(void);
void SystemClock_Config_LP(void);
static void MX_GPIO_Init(void);



typedef enum{RUN_LOW_POWER=0,
  CHANGE_LOW_POWER,
  CHANGE_HIGH_POWER,
  RUN_HIGH_POWER
}PowerState;

PowerState State;
uint8_t request_IRQ_ChangePower;

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config_LP();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();


  State=RUN_LOW_POWER;



  while (1)
  {
    switch (State)
    {

    case RUN_LOW_POWER:
    {
      /*Run in LowPower*/
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
      HAL_Delay(700);

      if (request_IRQ_ChangePower != 0U)
      {
        request_IRQ_ChangePower = 0U;
        State=CHANGE_HIGH_POWER;
      }

      break;

    }
    case CHANGE_LOW_POWER:
    {
      SystemClock_Config_LP();
      State=RUN_LOW_POWER;


      /*Correctif*/
      HAL_Delay(100);
      EXTI->IMR |= EXTI_IMR_MR0;

      break;

    }
    case CHANGE_HIGH_POWER:
    {
      SystemClock_Config_HP();
      State=RUN_HIGH_POWER;

      /*Correctif*/
      HAL_Delay(100);
      EXTI->IMR |= EXTI_IMR_MR0;

      break;

    }

    case RUN_HIGH_POWER:
    {
      /*Run in LowPower*/
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
      HAL_Delay(300);
      if (request_IRQ_ChangePower != 0U)
      {
        request_IRQ_ChangePower = 0U;
        State=CHANGE_LOW_POWER;
      }


      break;
    }

  }

}
/* USER CODE END 3 */
}

uint8_t nbreIrq;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin==GPIO_PIN_0)
  {
    /*EXTI0_IRQ*/
    request_IRQ_ChangePower=1U;
    nbreIrq++;

    /*Debounce Correction */
    EXTI->IMR &= ~EXTI_IMR_MR0;


  }
  else
  {
    /*EXTI1_IRQ*/

  }
}


/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config_HP(void)
{
RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

/** Configure the main internal regulator output voltage
 */
__HAL_RCC_PWR_CLK_ENABLE();
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

/** Initializes the RCC Oscillators according to the specified parameters
 * in the RCC_OscInitTypeDef structure.
 */
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_ON;
RCC_OscInitStruct.HSIState = RCC_HSI_ON;
RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
RCC_OscInitStruct.PLL.PLLM = 4;
RCC_OscInitStruct.PLL.PLLN = 168;
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
RCC_OscInitStruct.PLL.PLLQ = 4;
if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
{
  Error_Handler();
}

/** Initializes the CPU, AHB and APB buses clocks
 */
RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
{
  Error_Handler();
}
HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_4);
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config_LP(void)
{
RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

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
RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
{
  Error_Handler();
}
HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_4);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = { 0 };
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

/* GPIO Ports Clock Enable */
__HAL_RCC_GPIOH_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();
__HAL_RCC_GPIOD_CLK_ENABLE();

/*Configure GPIO pin Output Level */
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13, GPIO_PIN_RESET);

/*Configure GPIO pin : PA0 */
GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/*Configure GPIO pin : PA1 */
GPIO_InitStruct.Pin = GPIO_PIN_1;
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
GPIO_InitStruct.Pull = GPIO_PULLUP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/*Configure GPIO pins : PD12 PD13 */
GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/*Configure GPIO pin : PA8 */
GPIO_InitStruct.Pin = GPIO_PIN_8;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* EXTI interrupt init*/
HAL_NVIC_SetPriority(EXTI0_IRQn, 4, 0);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);

HAL_NVIC_SetPriority(EXTI1_IRQn, 7, 0);
HAL_NVIC_EnableIRQ(EXTI1_IRQn);

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
