/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "FreeRTOS.h"
#include "task.h"

#include "date_and_time.h"

int iTimeZone;

uint32_t ulSeconds, ulMsec;

/*
 * You can add the following code to you FreeRTOSConfig file:
 *
	extern TickType_t ulSeconds, ulMsec;

	#define traceINCREASE_TICK_COUNT( xTicksToJump ) \
	{ \
		ulMsec += xTicksToJump; \
		if( ulMsec >= 1000 ) \
		{ \
			ulSeconds += ( ulMsec / 1000ul ); \
			ulMsec = ( ulMsec % 1000ul ); \
		} \
	}


	#define traceTASK_INCREMENT_TICK( xTickCount ) \
	if( uxSchedulerSuspended == ( UBaseType_t ) pdFALSE ) \
	{ \
		if( ++ulMsec >= 1000 ) \
		{ \
			ulMsec = 0; \
			ulSeconds++; \
		} \
	}
 
 */


time_t FreeRTOS_time( time_t *pxTime )
{
time_t uxTime;

 	/* Critical section required if running on a 16 bit processor. */
//	portTICK_TYPE_ENTER_CRITICAL();
	{
		uxTime = ( time_t ) ulSeconds;
	}
//	portTICK_TYPE_EXIT_CRITICAL();
	if( pxTime != NULL )
	{
		*pxTime = uxTime;
	}
	return uxTime;
}
/*-----------------------------------------------------------*/

void FreeRTOS_settime( time_t *pxTime )
{
 	/* Critical section required if running on a 16 bit processor. */
//	portTICK_TYPE_ENTER_CRITICAL();
	{
		ulSeconds = ( uint32_t ) *pxTime;
		ulMsec = ( uint32_t ) 0;
	}
//	portTICK_TYPE_EXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

time_t FreeRTOS_get_secs_msec( time_t *pulMsec )
{
time_t uxReturn;

 	/* Critical section required if running on a 16 bit processor. */
//	portTICK_TYPE_ENTER_CRITICAL();
	{
		uxReturn = ( time_t ) ulSeconds;
		if( pulMsec != NULL )
		{
			*pulMsec = ulMsec;
		}
	}
//	portTICK_TYPE_EXIT_CRITICAL();

	return uxReturn;
}
/*-----------------------------------------------------------*/


void FreeRTOS_set_secs_msec( time_t *pulSeconds, time_t *pulMsec )
{

 	/* Critical section required if running on a 16 bit processor. */
//	portTICK_TYPE_ENTER_CRITICAL();
	{
		ulSeconds= *pulSeconds;
		if( pulMsec != NULL )
		{
			ulMsec = *pulMsec;
		}
	}
//	portTICK_TYPE_EXIT_CRITICAL();
}
/*-----------------------------------------------------------*/
