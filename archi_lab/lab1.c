/**
  ******************************************************************************
  * @file           : gpio.c
  * @brief          : lab1 gpio using pointer
  ******************************************************************************
/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#ifdef LAB1

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void delay(int x);

#define SET_LED1  *GPIOD_odr|=1<<12
#define SET_LED2  *GPIOD_odr=1<<13
#define SET_LED3  *GPIOD_odr=1<<14
#define SET_LED4  *GPIOD_odr=1<<15

//#define RESET_LED1    *GPIOD_odr&=0xEFFF
#define RESET_LED1  *GPIOD_odr&=~(1<<12)
#define RESET_LED2  *GPIOD_odr&=~(1<<13)
#define RESET_LED3  *GPIOD_odr&=~(1<<14)
#define RESET_LED4  *GPIOD_odr&=~(1<<15)

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
  *RCC_AHBR1ENR|=0x8;
  /*Config GPIOD en mode output*/
  *GPIOD_MODER|=0x55000000;

  while (1)
  {
   *GPIOD_odr =0xF000;
   delay(0xf0000);
   *GPIOD_odr =0;
   delay(0xf0000);
  }

}


void delay(int a)
{
  for (;a>0;a--)
  {
  }
}

#endif



