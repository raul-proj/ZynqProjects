#ifndef CCU_GIC_H
#define CCU_GIC_H

/******************************************************************************
 Includes
******************************************************************************/

#include "xscugic.h"
#include "xstatus.h"

/******************************************************************************
 Constant and macro definitions
******************************************************************************/
//Interrupt configuration for the peripherals in core#0
//Watchdog peripheral
#define WDG_INTERRUPT_ID                XPAR_SCUWDT_INTR    //!< Watchdog interrupt ID.
#define WDG_INTERRUPT_PRIORITY          0                   //!< Watchdog interrupt priority (timer mode).
#define WDG_INTERRUPT_TYPE              1                   //!< Active level of watchdog timer interrupts (rising edge).

//Processing system GPIO peripheral
#define GPIOPS_INTERRUPT_ID             XPAR_XGPIOPS_0_INTR //!< Processing system GPIO interrupt ID.
#define GPIOPS_INTERRUPT_PRIORITY       (configMAX_API_CALL_INTERRUPT_PRIORITY + 1) << portPRIORITY_SHIFT
#define GPIOPS_INTERRUPT_TYPE           0                   //!< Active level of PS GPIO interrupts (active high).

//I2C0 peripheral
#define I2C_INTERRUPT_ID        		XPAR_XIICPS_0_INTR  //!< IRQ from PS
#define I2C_INTERRUPT_PRIORITY			(configMAX_API_CALL_INTERRUPT_PRIORITY + 3) << portPRIORITY_SHIFT
												            //!< I2C0 interrupt priority
#define I2C_INTERRUPT_TYPE      		0		       		//!< Active level used when interrupts arrives to the GIC

//QSPI peripheral
#define QSPI_INTERRUPT_ID 				XPAR_XQSPIPS_0_INTR //!< QSPI0 interrupt ID
#define QSPI_INTERRUPT_PRIORITY			(configMAX_API_CALL_INTERRUPT_PRIORITY + 4) << portPRIORITY_SHIFT
												            //!< QSPI0 interrupt priority
#define QSPI_INTERRUPT_TYPE				0		            //!< Active level used when interrupts arrives to the GIC

//SPI peripheral
#define SPI_INTERRUPT_ID 				XPAR_XSPIPS_0_INTR	//!< SPI0 interrupt ID
#define SPI_INTERRUPT_PRIORITY			(configMAX_API_CALL_INTERRUPT_PRIORITY + 2) << portPRIORITY_SHIFT
												       		//!< SPI0 interrupt priority
#define SPI_INTERRUPT_TYPE				0		       		//!< Active level used when interrupts arrives to the GIC

//Anybus Profinet brick
#define ANYBUS_INTERRUPT_ID				XPS_FPGA3_INT_ID	//<! PL Shared interrupt
#define ANYBUS_INTERRUPT_PRIORITY		(configMAX_API_CALL_INTERRUPT_PRIORITY + 2) << portPRIORITY_SHIFT
#define ANYBUS_INTERRUPT_TYPE			1					//<! Edge type interrupt

// MeasADC peripheral interrupt
//#define MEASADC_INTERRUPT_ID 			XPS_FPGA0_INT_ID     //!< IRQ from PL (SGI Interrupt)
#define MEASADC_INTERRUPT_ID 			XPAR_FABRIC_MEASADC_0_IRQ1_INTR     //!< IRQ from PL (PPI Interrupt)
#define MEASADC_INTERRUPT_PRIORITY 		(configMAX_API_CALL_INTERRUPT_PRIORITY + 1) << portPRIORITY_SHIFT
//#define MEASADC_INTERRUPT_TYPE			1				//!< Rising edge used when SGI interrupts arrives to the GIC
#define MEASADC_INTERRUPT_TYPE			0					//!< Active level used when PPI interrupts arrives to the GIC

//Software interrupt from control core (core#1)
#define CONTROL_SW_INTERRUPT_PRIORITY	configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT

/******************************************************************************
 Data Types
******************************************************************************/

/******************************************************************************
 Global Variables
******************************************************************************/

/******************************************************************************
 Public function declarations
******************************************************************************/

XStatus GIC_Init(void);
XStatus GIC_SetInterrupt(uint32_t Int_Id, uint8_t Priority, uint8_t Trigger,
        Xil_ExceptionHandler InterruptHandler, void *CallBackRef);
XStatus GIC_SetInterruptHandler(uint32_t Int_Id, Xil_ExceptionHandler InterruptHandler, void *CallBackRef);
void GIC_ClearPendingInterrupts(void);
void GIC_EnableInterrupt(uint32_t Int_Id);
void GIC_DisableInterrupt(uint32_t Int_Id);

#endif /* CCU_GIC_H */
