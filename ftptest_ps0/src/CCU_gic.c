/******************************************************************************
 Includes
******************************************************************************/

#include "FreeRTOS.h"

#include "xparameters.h"

#include "CCU_gic.h"
#include "CCU_assert.h"

/******************************************************************************
 Constant and macro definitions
******************************************************************************/

/******************************************************************************
 Data Types
******************************************************************************/

/******************************************************************************
 Local Variables
******************************************************************************/

static XScuGic* pxGicInstance;         //!< Pointer to XScuGic driver instance data.

/******************************************************************************
 Global Variables
******************************************************************************/

extern XScuGic xInterruptController;  //!< Interrupt controller instance; defined by FreeRTOS in portZynq7000.c.

/******************************************************************************
 Local function declarations
******************************************************************************/

/******************************************************************************
 Public function definitions
******************************************************************************/

/**************************************************************************//**
*  Routine:     GIC_Init
*  @brief       Initializes the Generic Interrupt Controller (GIC).
*
*  @return      Status value code contained in xstatus.h.
*  @retval      XST_SUCCESS if the initialization has finished correctly.
*  @retval      XST_FAILURE if there is some problem during initialization.
******************************************************************************/
XStatus GIC_Init(void)
{
    XStatus xStatus;
    XScuGic_Config* pxGicConfig;

    pxGicInstance = &xInterruptController;

    /* Initialize the interrupt controller driver so that it is ready to use */
    pxGicConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    if (pxGicConfig == NULL)
    {
        xil_printf("Core#%d: ERROR initializing the interrupt controller driver.\r\n", XPAR_CPU_ID);
        return XST_FAILURE;
    }

    /* Sanity check the FreeRTOSConfig.h settings are correct for the hardware. */
    configASSERT( pxGicConfig );
    configASSERT( pxGicConfig->CpuBaseAddress == ( configINTERRUPT_CONTROLLER_BASE_ADDRESS + configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET ) );
    configASSERT( pxGicConfig->DistBaseAddress == configINTERRUPT_CONTROLLER_BASE_ADDRESS );

    /* Initialize the SCU and GIC to enable the desired interrupt configuration */
    xStatus = XScuGic_CfgInitialize(pxGicInstance, pxGicConfig, pxGicConfig->CpuBaseAddress);
    if (xStatus != XST_SUCCESS)
    {
        xil_printf("Core#%d: ERROR initializing the SCU and GIC.\r\n", XPAR_CPU_ID);
        return xStatus;
    }

    /* Initialize the exception table and register the interrupt controller
     * handler with the exception table. */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, pxGicInstance);

    /* Enable interrupts in the Processor. */
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

/**************************************************************************//**
*  Routine:     GIC_SetInterrupt
*  @brief       Configures an interrupt in the Generic Interrupt Controller.
*               Sets its priority and trigger type, associates a ISR with the
*               interrupt source and enables the interrupt.
*
*  @param[in]   Int_Id is the ID of the interrupt source to be configured
*               (IDs are defined in xparameters_ps.h).
*  @param[in]   Priority is the priority of the interrupt source
*               (from 0, highest, to 0xF8, lowest, in steps of 8).
*  @param[in]   Trigger is the trigger type of the interrupt source.
*  @param[in]   InterruptHandler is a pointer to the ISR associated with the
*               interrupt source.
*  @param[in]   CallBackRef is a parameter used to pass extra information to
*               the ISR (usually the instance pointer of a peripheral driver).
*
*  @return      Status value code contained in xstatus.h.
*  @retval      XST_SUCCESS if the interrupt has been configured correctly.
*  @retval      XST_FAILURE if there is some problem configuring the interrupt.
******************************************************************************/
XStatus GIC_SetInterrupt(uint32_t Int_Id, uint8_t Priority, uint8_t Trigger,
        Xil_ExceptionHandler InterruptHandler, void *CallBackRef)
{
    XStatus xStatus;
    uint32_t Mask;

    // Set the priority of the monitor interrupt
    XScuGic_SetPriorityTriggerType(pxGicInstance, Int_Id, Priority, Trigger);

    xStatus = XScuGic_Connect(pxGicInstance, Int_Id, InterruptHandler, CallBackRef);
    AssertNonvoid(xStatus != XST_SUCCESS);

#ifndef RELEASE_VERSION
    // Clean the pending interrupt in GIC of the current source if any
    Mask = 0x00000001U << (Int_Id % 32U);

    if (XScuGic_DistReadReg(pxGicInstance, (u32)XSCUGIC_ACTIVE_OFFSET + ((Int_Id / 32U)* 4U)) & Mask)
    {
        XScuGic_CPUWriteReg(pxGicInstance, XSCUGIC_EOI_OFFSET, Int_Id);
    }
#endif

    //Enables the routing of the interrupt to the CPU in the distributor
    XScuGic_Enable(pxGicInstance, Int_Id);

    return XST_SUCCESS;
}

/**************************************************************************//**
*  Routine:     GIC_SetInterruptHandler
*  @brief       Associates a ISR with a specific interrupt source.
*
*  @param[in]   Int_Id is the ID of the interrupt source to be configured
*               (IDs are defined in xparameters_ps.h).
*  @param[in]   InterruptHandler is a pointer to the ISR associated with the
*               interrupt source.
*  @param[in]   CallBackRef is a parameter used to pass extra information to
*               the ISR (usually the instance pointer of a peripheral driver).
*
*  @return      Status value code contained in xstatus.h.
*  @retval      XST_SUCCESS if the ISR is correctly assigned to the interrupt.
*  @retval      XST_FAILURE if the ISR is not assigned to the interrupt.
******************************************************************************/
XStatus GIC_SetInterruptHandler(uint32_t Int_Id, Xil_ExceptionHandler InterruptHandler, void *CallBackRef)
{
    XStatus xStatus;
    xStatus = XScuGic_Connect(pxGicInstance, Int_Id, InterruptHandler, CallBackRef);
    AssertNonvoid(xStatus != XST_SUCCESS);
    return XST_SUCCESS;
}

/**************************************************************************//**
*  Routine:     GIC_ClearPendingInterrupts
*  @brief       Clear any SGI or PPI pending interrupt in the GIC. The cleaning
*               of the pending interrupts is only possible when there are no
*               active interrupts and the interrupt sources are not active.
*
*  @return      None
******************************************************************************/
void GIC_ClearPendingInterrupts(void)
{
    int i = 0;
    uint32_t Int_Id;

    //If any SGI is in the active and pending state, it is not possible to drop. A reset is required
    if ((XScuGic_DistReadReg(pxGicInstance, (u32)XSCUGIC_ACTIVE_OFFSET) & 0xFFFF) &&
                            (XScuGic_DistReadReg(pxGicInstance, (u32)XSCUGIC_ACTIVE_OFFSET) & 0xFFFF))
    {
        xil_printf("Core#0: SGI locked state\r\n");
        return;
    }

    //Clean any interrupt if pending in the GIC
    do {
        Int_Id = XScuGic_CPUReadReg(pxGicInstance, XSCUGIC_INT_ACK_OFFSET);
        XScuGic_CPUWriteReg(pxGicInstance, XSCUGIC_EOI_OFFSET, Int_Id);
        i++;
    }
    while((Int_Id != 0x3FF) && (i < XSCUGIC_MAX_NUM_INTR_INPUTS));
}

/**************************************************************************//**
*  Routine:     GIC_EnableInterrupt
*  @brief       Enables an interrupt source in the GIC.
*
*  @param[in]   Int_Id is the ID of the interrupt source to be enabled
*               (IDs are defined in xparameters_ps.h).
*
*  @return      None.
******************************************************************************/
void GIC_EnableInterrupt(u32 Int_Id)
{
    XScuGic_Enable(pxGicInstance, Int_Id);
}

/**************************************************************************//**
*  Routine:     GIC_DisableInterrupt
*  @brief       Disables an interrupt source in the GIC.
*
*  @param[in]   Int_Id is the ID of the interrupt source to be disabled
*               (IDs are defined in xparameters_ps.h).
*
*  @return      None.
******************************************************************************/
void GIC_DisableInterrupt(u32 Int_Id)
{
    XScuGic_Disable(pxGicInstance, Int_Id);
}

/******************************************************************************
 Local function definitions
******************************************************************************/
