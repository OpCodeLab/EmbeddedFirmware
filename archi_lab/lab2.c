/**
  ******************************************************************************
  * @file           : gpio.c
  * @brief          : lab2 gpio using cmsis
  ******************************************************************************
/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#ifdef LAB2

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void delay(int x);

#define SET_LED1   GPIOD->ODR = 1<<12
#define SET_LED2   GPIOD->ODR = 1<<13
#define SET_LED3   GPIOD->ODR = 1<<14
#define SET_LED4   GPIOD->ODR = 1<<15

//#define RESET_LED1    *GPIOD_odr&=0xEFFF
#define RESET_LED1   GPIOD->ODR &= ~(1<<12)
#define RESET_LED2   GPIOD->ODR &= ~(1<<13)
#define RESET_LED3   GPIOD->ODR &= ~(1<<14)
#define RESET_LED4   GPIOD->ODR &= ~(1<<15)

int *RCC_AHBR1ENR = (int*)0x40023830;
int *GPIOD_MODER =  (int*)0x40020C00;
int *GPIOD_odr   =  (int*)0x40020C14;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{


  /*Enable GPIOD via RCC*/   // 00001000
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

  /*Config GPIOD en mode output*/
  GPIOD->MODER |= 0x55000000;


  while (1)
  {
    SET_LED1;
    delay(0xFF000);
    SET_LED2;
    delay(0xFF000);
    SET_LED3;
    delay(0xFF000);
    SET_LED4;
    delay(0xFF000);
  }

}


void delay(int a)
{


  for (;a>0;a--)
  {

  }

}



#endif


