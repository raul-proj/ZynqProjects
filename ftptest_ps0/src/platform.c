/******************************************************************************
 Includes
******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* Xilinx includes */
#include "xil_mmu.h"
#include "xil_cache_l.h"    // Xil_DCacheFlushLine
//#include "xil_cache.h"


#include "platform.h"

#include "tcpip_api_init.h"
#include "CCU_gic.h"


/******************************************************************************
 Constant and macro definitions
******************************************************************************/
// Wake up core#1
#define sev() __asm__("sev")
#define CPU1_MEM_BASE        			0x1010012c
#define CPU1_START_ADR       			0xFFFFFFF0

/******************************************************************************
 Data Types
******************************************************************************/

/******************************************************************************
* Local Variables
******************************************************************************/

/******************************************************************************
 Global Variables
******************************************************************************/
// Global variables should be avoided

/******************************************************************************
 Declaration of Local Functions
******************************************************************************/


/******************************************************************************
*
* Definition of Global Functions
*
******************************************************************************/


/**************************************************************************//**
*  Routine:     wakeup_core1
*  @brief       Start application on core#1
*
*
*  @return      None
******************************************************************************/
void wakeup_core1(void)
{
    xil_printf("Core#0: : Setting start address for Core#1 to 0x%08X\n\r", CPU1_MEM_BASE);
    // Write the address of the application for CPU 1 to 0xFFFFFFF0
    Xil_Out32(CPU1_START_ADR, CPU1_MEM_BASE);
    dmb();          // Wait until memory write has finished.
    Xil_DCacheFlushLine(CPU1_START_ADR);

    xil_printf("Core#0: : Using SEV to wake up Core#1\n\r");
    // Execute the SEV instruction to cause CPU 1 to wake up and jump to the application
    sev();
}

/**************************************************************************//**
*  Routine:     disable_cache_ocm
*  @brief       Disable L1 cache on OCM
*
*
*  @return      None
******************************************************************************/
void disable_cache_ocm(void)
{
	//Cache disabled in OCM memory space that allocates the ADC Measurement buffer
//	Xil_SetTlbAttributes((u32)ADC_MEAS_BUFFER_ADDR,NORM_NONCACHE);
}

/**************************************************************************//**
*  Routine:     init_hw_platform
*  @brief       Initializes BSP drivers used by application
*
*
*  @return      Status value code contained in xstatus.h
******************************************************************************/
XStatus init_hw_platform(void)
{
    XStatus xStatus = XST_SUCCESS;

    // TODO: Add the initialization required by the hardware application

    // 1. Disable IRQ interrupts if enabled
    Xil_ExceptionDisable();

    // 2. Disable L1 cache on OCM
    disable_cache_ocm();

    // 3. Initialize interrupt system controller (GIC)
    xStatus = GIC_Init();
    configASSERT(xStatus == XST_SUCCESS);

#ifndef RELEASE_VERSION
    // 9. Clear any SGI/PPI pending interrupt if any
    GIC_ClearPendingInterrupts();
#endif

    // 10. Initialize XEmacPs Driver
    xStatus = init_xemacps(start_network_applications);
    configASSERT(xStatus == XST_SUCCESS);

    return xStatus;
}

/******************************************************************************
*
* Definition of Local Functions
*
******************************************************************************/

