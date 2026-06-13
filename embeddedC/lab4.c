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

#ifdef LAB4
int function (int a, int b);
void weakfunc(void ) __attribute__((weak));

extern void initialise_monitor_handles(void);
int object=45;
int main_global=6;

static main_staticvar=4;

#define CORRECT

void weakfunc(void )
{
   /*Default implementation */
  object++;
}

int16_t i16= -4;

uint8_t u8= 255;

uint32_t u32=0xFFFF;

uint16_t u16=15555;
float y;
uint32_t u32result;
uint16_t u16result;
uint8_t  u8result;
float x;
int8_t i = -1;
uint16_t limit = 200U;
char buffer[255];
int main(void)
{
  //int z=function(main_global,main_staticvar);
  initialise_monitor_handles();
  uint8_t aa=5;
  uint8_t b=2;
  printf("Please enter your name: \n");
  FILE *fd= fopen("D:\\test.out", "w+");
  if(fd) {
      fwrite(buffer, sizeof(char), strlen(buffer), fd);
      fclose(fd);
  }

#ifdef CORRECT















  uint16_t u = 100;
  int32_t s = -500;

  uint32_t result = u + s;                                          //u is promoted to int32_ before addition
                                                              // then the result is casted in uint32_t

  if (u<s)
  {
    result++;

  }















#elif defined INCORRECT
  uint32_t u = 100;
  int16_t s = -500;

  uint32_t result = u + s; //s is promoted to uint32_t
                              // then the result is casted in uint32_t

  if (u<s)             // //s is promoted to uint32_t  !!!!!!
  {
    result++;

  }
#elif defined INCORRECT2

  uint32_t u = 100;
  int32_t s = -500;

  uint32_t result = u + s; //s is promoted to uint32_t
                              // then the result is casted in uint32_t

  if (u<s)             // //s is promoted to uint32_t  !!!!!!
  {
    result++;

  }
#else
//  #elif defined INCORRECT2

  uint16_t u16=200;
  uint8_t u8 = 100;
  uint32_t u32 = 100;
  int8_t s8 = -50;
  int16_t s16=-40;
  int32_t s32=-80;

  //u32(200)+s16(-400) == >  (cast u32)-200

  uint32_t result = u32 + s8+u8+s16+s32+u16; //u & s are promoted to int32_t
                              // then the result is casted in uint32_t

  if (u32<s32)             // //u is promoted to int32_t  !!!!!!
  {
    result++;

  }
  if (u32<s16)             // //u is promoted to int16_t  !!!!!!
  {
    result++;

  }
  if (u32<s8)             // //u is promoted to int16_t  !!!!!!
  {
    result++;

  }

  if (u16<s32)             // //u is promoted to int16_t  !!!!!!
  {
    result++;

  }
  if (u16<s16)             // //u is promoted to int16_t  !!!!!!
  {
    result++;

  }
  if (u16<s8)             // //u is promoted to int16_t  !!!!!!
  {
    result++;
  }

  if (u8<s32)             // //u is promoted to int16_t  !!!!!!
  {
    result++;

  }
  if (u8<s16)             // //u is promoted to int16_t  !!!!!!
  {
    result++;

  }
  if (u8<s8)             // //u is promoted to int16_t  !!!!!!
  {
    result++;

  }





#endif


  y=  aa/(float)b;    ///// !!!!
  float fa=5;
  y= fa/b;
  y=  (float)aa/b;    // correct  implicit type convertion

  int8_t i = -1;
  uint16_t limit = 200U; ///
  long n = 30L;

  if ( i < limit )
     x = limit * n;

  weakfunc();

  while(1);

}

#endif
