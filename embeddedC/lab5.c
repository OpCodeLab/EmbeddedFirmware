/**
  ******************************************************************************
  * @file           : lab3.c
  * @brief          : typecase & var boundary
  ******************************************************************************
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"

#ifdef LAB5


extern void initialise_monitor_handles(void);

void   funct(void);

__attribute__((__section__(".mysection")))  uint8_t buffer[20U] ;



int main(void)
{
  initialise_monitor_handles();

  funct();

  uint8_t aa=buffer[0];
  uint8_t b=2;
  printf("Please enter your name: \n");
  FILE *fd= fopen("D:\\test.out", "w+");
  if(fd) {
      fwrite("hello", sizeof(char), 5, fd);
      fclose(fd);
  }

}

void __attribute__((optimize("O0"))) foo(unsigned char data) {
    // unmodifiable compiler code
}


 __attribute__((optimize("O0"))) __attribute__((section(".mysection_text"))) void  funct(void)
{
  buffer[0]=5;
  for (uint16_t i=0;i<10000;i++);

  buffer[1]=50;

}


#endif
