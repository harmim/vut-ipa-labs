/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
#include "stm32f4xx_conf.h"

extern void beep();

int main(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    GPIO_InitTypeDef GPIO_InitDef;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT; //(GPIO_Mode_AF|GPIO_Mode_OUT)
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_DOWN; //(GPIO_PuPd_UP|GPIO_PuPd_NONE)
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitDef);


    GPIO_InitTypeDef GPIO_InitDefx;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitDefx.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitDefx.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitDefx.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDefx.GPIO_PuPd = GPIO_PuPd_UP; //(GPIO_PuPd_UP|GPIO_PuPd_NONE)
    GPIO_InitDefx.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitDefx);


    beep();
}
