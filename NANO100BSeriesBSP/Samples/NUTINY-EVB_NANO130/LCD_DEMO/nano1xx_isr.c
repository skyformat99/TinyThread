/******************************************************************************
 * @file     nano1xx_isr.c
 * @brief    This file contains interrupt service routines
 * @version  1.0.1
 * @date     04, September, 2012
 *
 * @note
 * Copyright (C) 2012-2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "nano1xx.h"
#include "nano1xx_rtc.h"
#include "nano1xx_lcd.h"


#ifdef __DEBUG_MSG
#define DEBUG_MSG 	printf 
#else
#define DEBUG_MSG(...)
#endif

extern	void showTime(uint32_t, uint32_t);
extern  void planNextRTCInterrupt(S_DRVRTC_TIME_DATA_T *);

__IO uint32_t g_u32RTC_Count  = 0;

/**
  * @brief  RTC IRQHandler. 
  * @param  None.
  * @return None.
  */
void RTC_IRQHandler()
{
 	S_DRVRTC_TIME_DATA_T sCurTime;

	DEBUG_MSG("RTC_IRQHandler running...\n");

	/* RTC Tick interrupt */
	if ((RTC->RIER & RTC_RIER_TIER) && (RTC->RIIR & RTC_RIIR_TIS))		
	{
	    DEBUG_MSG("RTC Tick Interrupt.\n");
		RTC->RIIR = RTC_RIIR_TIS;

		if((g_u32RTC_Count %2 ) == 0)
			LCD_DisableSegment(3, 29);
		else
			LCD_EnableSegment(3, 29);

		g_u32RTC_Count++;  /* increase RTC tick count */

    }

	/* RTC Alarm interrupt */
	if ((RTC->RIER & RTC_RIER_AIER) && (RTC->RIIR & RTC_RIIR_AIS))		
    {          
		DEBUG_MSG("RTC Alarm Interrupt.\n");
		RTC->RIIR = RTC_RIIR_AIS;
		  
		RTC_Read(&sCurTime);
		DEBUG_MSG("Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
		showTime(sCurTime.u32cHour, sCurTime.u32cMinute);

	    sCurTime.u8IsEnableWakeUp = 0;	/* RTC tick shouldn't wake up CPU */
		planNextRTCInterrupt(&sCurTime);
	}	

	if ((RTC->RIER & RTC_RIER_SNOOPIER) && (RTC->RIIR & RTC_RIIR_SNOOPIS))	/* snooper interrupt occurred */
    {          
		  RTC->RIIR = RTC_RIIR_SNOOPIS;
	}

}

/*** (C) COPYRIGHT 2012 Nuvoton Technology Corp. ***/
