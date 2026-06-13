/**
  ******************************************************************************
  * @file           : module1.c
  * @brief          : module1
  ******************************************************************************
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "module1.h"
#include "string.h"

#ifdef MODULE1

//char ch1[]="{\"gateId\":\"12345\",\"params\":{\"id\":\"83945071\",\"cmd\":\"c\",\"dg\":\"11110010\",\"aid\":\"1\",\"aval\":\"1234\"}}.";

enum {PSTATE_WAIT_PATTERN, PSTATE_WAIT_PATTERN2,PSTATE_RECORDING, PSTATE_EXTRACT_DONE,PSTATE_ERROR_OVERFLOW};

#define KEY_MAX_SIE           20U



static uint8_t Get_KeyValue(char *ch, uint16_t len, char *patternStart, uint16_t pattSize, char *key, uint16_t keySize);
static uint8_t Get_ValueFromKey(char *ch, uint16_t len, char *key, uint16_t sizeKey,uint32_t *val);
static uint8_t ChToVal(char *ch, uint16_t len, uint32_t *val);




/**
 *
 */
uint8_t Get_GatewayID(char *ch, uint16_t len, uint32_t *gtw)
{
  char key[]="gateId\":";
  uint8_t ret;

  ret =Get_ValueFromKey(ch,len,key, strlen(key),gtw );

 return ret;
}
/**
 *
 */
uint8_t Get_End_devID(char *ch, uint16_t len, uint32_t *edvid)
{
 char key[]="id\":";
  uint8_t ret;
 ret =Get_ValueFromKey(ch,len,key, strlen(key),edvid );

 return ret;

}


/**
 *
 */
static uint8_t Get_KeyValue(char *ch, uint16_t len, char *patternStart, uint16_t pattSize, char *key, uint16_t keySize)
{
    uint8_t ret;
    char tmp[KEY_MAX_SIE];
    uint8_t state=PSTATE_WAIT_PATTERN;
    uint8_t j=0;  uint8_t patpos=0;

    /*reset tmp */
    for (uint8_t i=0; i<KEY_MAX_SIE; i++)
    {
        tmp[i]=0;
        key[i]=0;
    }

    for (uint16_t i=0; i<len; i++)
    {
        switch(state)
        {
            case PSTATE_WAIT_PATTERN:
            {
              if ( (ch[i]==patternStart[patpos])  )
               {
                 state=PSTATE_WAIT_PATTERN2;
                 patpos++;
               }
               break;
            }
            case PSTATE_WAIT_PATTERN2:
            {
               if (patpos<pattSize)
               {
                if ( (ch[i]==patternStart[patpos])  )
                 {
                    /*Continue Fetching ---*/
                    patpos++;
                 }
                 else
                 {
                     /*Fail for fech */
                     patpos=0;
                     state=PSTATE_WAIT_PATTERN;
                     break;
                 }
               }
               else
               {
                 state=PSTATE_RECORDING;
               }
               break;
            }

            case PSTATE_RECORDING:
            {
                if (j>KEY_MAX_SIE)
                {
                  state=PSTATE_ERROR_OVERFLOW;
                  break;
                }
                else
                {
                    if (ch[i]=='"')
                    {
                     state=PSTATE_EXTRACT_DONE;
                     break;
                    }
                    else
                    {
                     tmp[j]=ch[i];
                     j++;
                    }
                }

                break;
            }

          case PSTATE_EXTRACT_DONE:
          case PSTATE_ERROR_OVERFLOW:
            {
               break;
            }

        }

        if((state==PSTATE_ERROR_OVERFLOW) || (state==PSTATE_EXTRACT_DONE))
        {
            break;
        }
    }


    if (state ==PSTATE_EXTRACT_DONE )
    {
       ret=0;
       /*Save the key */
       for (uint8_t i=0;((i<KEY_MAX_SIE)&&(i<keySize));i++)
       {
           key[i]=tmp[i];
       }

    }
    else
    {
        ret =1;
    }
    return ret;
}


static uint8_t Get_ValueFromKey(char *ch, uint16_t len, char *key, uint16_t sizeKey,uint32_t *val)
{
  static char value[KEY_MAX_SIE];
  uint8_t ret=Get_KeyValue(ch,len,key,sizeKey,value,KEY_MAX_SIE);

  if (ret==0)
  {
     ret= ChToVal(value,KEY_MAX_SIE,val);
  }

 return ret;

}


static uint8_t ChToVal(char *ch, uint16_t len, uint32_t *val)
{
    uint8_t ret=0;
    uint32_t tmp=0;

    for (uint8_t i=0; i<len ; i++)
    {
        if ((ch[i]>='0') && (ch[i]<='9'))
        {
            tmp=tmp*10 + ch[i]-'0';
        }
        else
        {
            if ((ch[i]==0) && (i >0 ))
            {
                /*end parsing -----*/
                ret=0;
                *val=tmp;
            }
            else
            {
              ret=1;
              break;
            }
        }
    }


return ret;
}

#endif /*MODULE1*/
