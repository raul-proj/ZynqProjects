/*  NOTE: Next setting MUST be done to the application project when FreeRTOS+TCP
 *        stack is added to an existing application.
 *
 *  Project->Properties:
 *      C/C++ Build -> Settings: Tool Settings view:
 *          ARM v7 gcc compiler -> Directories:
 *
 *      Include Paths, for All Configurations add:
 *
 *      "${workspace_loc:/${ProjName}/src/service/tcpip}"
 *      "${workspace_loc:/${ProjName}/src/service/tcpip/FreeRTOS-Plus-TCP/include}"
 *      "${workspace_loc:/${ProjName}/src/service/tcpip/FreeRTOS-Plus-TCP/portable/Compiler/GCC}"
 *      "${workspace_loc:/${ProjName}/src/service/tcpip/FreeRTOS-Plus-TCP/portable/NetworkInterface}"
 */

/******************************************************************************
 Includes
******************************************************************************/
/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
/* Xilinx includes */
#include "xil_printf.h"
#include "xstatus.h"
#include "xtime_l.h"
#include "xil_mem.h"
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"

#include "FreeRTOS_TCP_server.h"

#include "tcpip_api_init.h"

#include "ff_ramdisk.h"

/******************************************************************************
 Constant and macro definitions
******************************************************************************/

/* Dimensions the arrays into which print messages are created. */
#define MAX_PRINT_STRING_LENGTH             255

/* Default IP address configuration */
#define DEFAULT_IP_ADDR0            192
#define DEFAULT_IP_ADDR1            168
#define DEFAULT_IP_ADDR2            1
#define DEFAULT_IP_ADDR3            10

/* Default netmask configuration */
#define DEFAULT_NET_MASK0           255
#define DEFAULT_NET_MASK1           255
#define DEFAULT_NET_MASK2           255
#define DEFAULT_NET_MASK3           0

/* Default gateway IP address configuration */
#define DEFAULT_GATEWAY_ADDR0       192
#define DEFAULT_GATEWAY_ADDR1       168
#define DEFAULT_GATEWAY_ADDR2       1
#define DEFAULT_GATEWAY_ADDR3       1

/* Default DNS server configuration
 * OpenDNS addresses are 208.67.222.222 and 208.67.220.220 */
#define DEFAULT_DNS_SERVER_ADDR0    208
#define DEFAULT_DNS_SERVER_ADDR1    67
#define DEFAULT_DNS_SERVER_ADDR2    222
#define DEFAULT_DNS_SERVER_ADDR3    222

/* Default MAC address configuration */
#define DEFAULT_MAC_ADDR0           0x00
#define DEFAULT_MAC_ADDR1           0x0a
#define DEFAULT_MAC_ADDR2           0x35
#define DEFAULT_MAC_ADDR3           0x00
#define DEFAULT_MAC_ADDR4           0x01
#define DEFAULT_MAC_ADDR5           0x03


/* The number and size of sectors that will make up the RAM disk. */
#define mainRAM_DISK_SECTOR_SIZE		512UL 									//<! Currently fixed sector size for the RAM Disk
#define mainRAM_DISK_SECTORS			( ( 5UL * 1024UL * 1024UL ) / mainRAM_DISK_SECTOR_SIZE ) //<! 5M bytes of RAM Disk file system
#define mainIO_MANAGER_CACHE_SIZE		( 15UL * mainRAM_DISK_SECTOR_SIZE )		//<! Size of the RAM Disk cache

#define mainRAM_DISK_NAME				"/ram"									//<! RAM Disk mount directory


/******************************************************************************
 Data Types
******************************************************************************/

/**
    @struct SNetworkParams_t
    @brief  Structure to store the network parameters
*/
typedef struct
{
    uint8_t u8_ip_addr[ipIP_ADDRESS_LENGTH_BYTES];      //!< IP address
    uint8_t u8_netmask_addr[ipIP_ADDRESS_LENGTH_BYTES]; //!< Netmask address
    uint8_t u8_gw_addr[ipIP_ADDRESS_LENGTH_BYTES];      //!< Gateway address
    uint8_t u8_dns_addr[ipIP_ADDRESS_LENGTH_BYTES];     //!< DNS server address
    uint8_t u8_mac_addr[ipMAC_ADDRESS_LENGTH_BYTES];    //!< MAC address
} SNetworkParams_t;



/******************************************************************************
 Local Variables
******************************************************************************/

/* The default IP and MAC address will be used if ipconfigUSE_DHCP is 0,
 * or if ipconfigUSE_DHCP is 1 but a DHCP server could not be contacted. */
static const uint8_t ucIPAddress[ipIP_ADDRESS_LENGTH_BYTES] =
                                            { DEFAULT_IP_ADDR0, DEFAULT_IP_ADDR1,
                                              DEFAULT_IP_ADDR2, DEFAULT_IP_ADDR3 };
static const uint8_t ucNetMask[ipIP_ADDRESS_LENGTH_BYTES] =
                                            { DEFAULT_NET_MASK0, DEFAULT_NET_MASK1,
                                              DEFAULT_NET_MASK2, DEFAULT_NET_MASK3 };
static const uint8_t ucGatewayAddress[ipIP_ADDRESS_LENGTH_BYTES] =
                                            { DEFAULT_GATEWAY_ADDR0, DEFAULT_GATEWAY_ADDR1,
                                              DEFAULT_GATEWAY_ADDR2, DEFAULT_GATEWAY_ADDR3 };
static const uint8_t ucDNSServerAddress[ipIP_ADDRESS_LENGTH_BYTES] =
                                            { DEFAULT_DNS_SERVER_ADDR0, DEFAULT_DNS_SERVER_ADDR1,
                                              DEFAULT_DNS_SERVER_ADDR2, DEFAULT_DNS_SERVER_ADDR3 };
const uint8_t ucMACAddress[ipMAC_ADDRESS_LENGTH_BYTES] =
                            { DEFAULT_MAC_ADDR0, DEFAULT_MAC_ADDR1, DEFAULT_MAC_ADDR2,
                            DEFAULT_MAC_ADDR3, DEFAULT_MAC_ADDR4, DEFAULT_MAC_ADDR5 };


static UBaseType_t  ulNextRand;    //!< Use by the pseudo random number generator.

static ENetworkState_t  e_network_state;    //!< Store the network state to be available to applications.

static TaskHandle_t            xTask;


static const struct xSERVER_CONFIG s_server_configuration[] =
{
        /* Server type,     port number,        backlog,                root directory */
#if ipconfigUSE_FTP == 1
        { eSERVER_FTP,      21,    				12,					    "/ram"    },
#endif
#if ipconfigUSE_HTTP == 1

#endif
};


static FF_Disk_t *pxRAMDisk;


/******************************************************************************
 Global Variables
******************************************************************************/
// Global variables should be avoided



/******************************************************************************
 Declaration of Local Functions
******************************************************************************/
static void tcpserver_task(void *pvParameters);
static XStatus File_System_Init(void);


/******************************************************************************
*
* Definition of Global Functions
*
******************************************************************************/

/**************************************************************************//**
*  Routine:     init_xemacps
*  @brief       This function initializes XEmacPs driver using FreeRTOS+TCP stack.
*
*  @param       *p_start_net_apps   For FreeRTOS+TCP must be NULL
*
*  @return      Status value code contained in xstatus.h
******************************************************************************/
int init_xemacps(void (*p_start_net_apps)(void))
{
    SNetworkParams_t    s_network_prm;
    BaseType_t          ret_value = pdFALSE;
    XTime               xtimenow;

    //1.- Get network parameters
    /* TODO: If there are not network parameters stored in Flash memory,
     * get them from the default values */
    memcpy(s_network_prm.u8_ip_addr, ucIPAddress, (size_t)ipIP_ADDRESS_LENGTH_BYTES);
    memcpy(s_network_prm.u8_netmask_addr, ucNetMask, (size_t)ipIP_ADDRESS_LENGTH_BYTES);
    memcpy(s_network_prm.u8_gw_addr, ucGatewayAddress, (size_t)ipIP_ADDRESS_LENGTH_BYTES);
    memcpy(s_network_prm.u8_dns_addr, ucDNSServerAddress, (size_t)ipIP_ADDRESS_LENGTH_BYTES);
    memcpy(s_network_prm.u8_mac_addr, ucMACAddress, (size_t)ipMAC_ADDRESS_LENGTH_BYTES);

    //2.- Initialize the network interface
    ret_value = FreeRTOS_IPInit(s_network_prm.u8_ip_addr, s_network_prm.u8_netmask_addr,
                                s_network_prm.u8_gw_addr, s_network_prm.u8_dns_addr,
                                s_network_prm.u8_mac_addr);
    // Check the FreeRTOS+TCP has been initialized
    configASSERT(ret_value == pdPASS);

    //3.- Seed the random number generator
    /* This is not a secure method of generating a random number.
     * It is a pseudo Random Number Generator.
     */
    XTime_GetTime(&xtimenow);
    FreeRTOS_printf( ("xtimenow: %08X %08X %08X %08X\n", xtimenow) );
    ulNextRand = (UBaseType_t)xtimenow;
    FreeRTOS_printf( ("Random numbers: %08X %08X %08X %08X\n",
                ipconfigRAND32(), ipconfigRAND32(), ipconfigRAND32(), ipconfigRAND32()) );

    //4.- Initialize network variables
    e_network_state = NETWORK_DOWN;

    return XST_SUCCESS;
}

/**************************************************************************//**
*  Routine:     get_network_state
*  @brief       This function returns the state of the network: NETWORK_UP or NETWORK_DOWN
*
*
*  @return      None
******************************************************************************/
ENetworkState_t get_network_state(void)
{
    return(e_network_state);
}


/**************************************************************************//**
*  Routine:     start_network_applications
*  @brief       This function is defined as part of the TCP/IP wrapper layer
*
*
*  @return      None
******************************************************************************/
void start_network_applications(void)
{
    /* NOTE: This function is defined as part of the TCP/IP wrapper layer to
     *       avoid change the platform.c file.
     *       This function is used by lwIP stack.
     */
}



/**************************************************************************//**
*  Routine:     uxRand
*  @brief       Generates a pseudo random number
*               This is not a secure method of generating a random number.
*
*  @return      The random number
******************************************************************************/
UBaseType_t uxRand(void)
{
    const uint32_t  ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;
    uint32_t        ulRandomValue = 0;

    /* Utility function to generate a pseudo random number. */
    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    ulRandomValue = (( ulNextRand >> 16UL ) & 0x7fffUL );

    return ulRandomValue;
}

/**************************************************************************//**
*  Routine:     xApplicationGetRandomNumber
*  @brief       Sets *pulNumber to a random number, and return pdTRUE. When
*               the random number generator is broken, it shall return pdFALSE.
*
*  @param       pulNumber [in][out] Pointer to variable where to store the random number.
*
*  @return      pdTRUE if successful, pdFALSE if not
******************************************************************************/
BaseType_t xApplicationGetRandomNumber( uint32_t *pulNumber )
{
    /*
     *  This implementation is based on uxRand() function used by several
     *  FreeRTOS+TCP Demo as FreeRTOS_Plus_TCP_Minimal_Windows_Simulator,
     *  \FreeRTOS-IoT-Libraries-LTS-Beta1 and \FreeRTOS-IoT-Libraries-LTS-Beta2.
     *
     *  This is not a secure method of generating a random number.
     *
     *  The seed is initialized with the value of the Global Timer Counter Register
     *  at each startup.
     */

    *pulNumber = uxRand();
    return pdTRUE;
}

/**************************************************************************//**
*  Routine:     ulApplicationGetNextSequenceNumber
*  @brief       Generates a randomized TCP Initial Sequence Number per RFC.
*               This function must be provided by the application builder.
*
*  @param       ulSourceAddress [in]
*  @param       usSourcePort [in]
*  @param       ulDestinationAddress [in]
*  @param       usDestinationPort [in]
*
*  @return
******************************************************************************/
uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                            uint16_t usSourcePort,
                                            uint32_t ulDestinationAddress,
                                            uint16_t usDestinationPort )
{
    /*
     *  This implementation is based on uxRand() function used by several
     *  FreeRTOS+TCP Demo as FreeRTOS_Plus_TCP_Minimal_Windows_Simulator,
     *  \FreeRTOS-IoT-Libraries-LTS-Beta1 and \FreeRTOS-IoT-Libraries-LTS-Beta2.
     *
     *  This is not a secure method of generating a random number.
     *
     *  The seed is initialized with the value of the Global Timer Counter Register
     *  at each startup.
     */

    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    return uxRand();
}

/**************************************************************************//**
*  Routine:     vLoggingPrintf
*  @brief       Function used to print out
*
*  @param       pcFormatString [in] Formatted message with respective arguments.
*
*  @return      None
******************************************************************************/
void vLoggingPrintf( const char * pcFormatString,
                    ... )
{
    va_list             args;
    char                cPrintString[MAX_PRINT_STRING_LENGTH];
    char                cOutputString[MAX_PRINT_STRING_LENGTH];
    char                *pcSource, *pcTarget, *pcBegin;
    unsigned int        ulIPAddress;
    const char          *pcTaskName;
    const char          *pcNoTask = "None";
    size_t              xLength, xLength2, rc;
    static BaseType_t   xMessageNumber = 0;

    /* There are a variable number of parameters: Initialize arguments */
    va_start(args, pcFormatString);

    /* Additional info to place at the start of the log. */
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        pcTaskName = pcTaskGetName(NULL);
    }
    else
    {
        pcTaskName = pcNoTask;
    }

    if(strcmp(pcFormatString, "\n") != 0)
    {
        xLength = snprintf(cPrintString, MAX_PRINT_STRING_LENGTH, "%lu %lu [%s] ",
                            xMessageNumber++,
                            ( unsigned long ) xTaskGetTickCount(),
                            pcTaskName);
    }
    else
    {
        xLength = 0;
        memset(cPrintString, 0x00, MAX_PRINT_STRING_LENGTH);
    }

    xLength2 = vsnprintf(cPrintString + xLength, MAX_PRINT_STRING_LENGTH - xLength,
                        pcFormatString, args);
    if( xLength2 < 0 )
    {
        /* Clean up. */
        xLength2 = MAX_PRINT_STRING_LENGTH - 1 - xLength;
        cPrintString[MAX_PRINT_STRING_LENGTH - 1] = '\0';
    }
    xLength += xLength2;

    /* Clean arguments list for the next time */
    va_end(args);

    /* For ease of viewing, copy the string into another buffer, converting
     * IP addresses to dot notation on the way. */
    pcSource = cPrintString;
    pcTarget = cOutputString;
    while( (*pcSource) != '\0' )
    {
        *pcTarget = *pcSource;
        pcTarget++;
        pcSource++;

        /* Look forward for an IP address denoted by 'ip'. */
        if( (isxdigit(pcSource[0]) != pdFALSE) && (pcSource[1] == 'i') && (pcSource[2] == 'p') )
        {
            *pcTarget = *pcSource;
            pcTarget++;
            *pcTarget = '\0';
            pcBegin = pcTarget - 8;

            while((pcTarget > pcBegin) && (isxdigit(pcTarget[-1 ]) != pdFALSE))
            {
                pcTarget--;
            }

            sscanf(pcTarget, "%8X", &ulIPAddress);
            rc = sprintf(pcTarget, "%lu.%lu.%lu.%lu",
                          (unsigned long)(ulIPAddress >> 24UL),
                          (unsigned long)(( ulIPAddress >> 16UL) & 0xffUL),
                          (unsigned long)(( ulIPAddress >> 8UL) & 0xffUL),
                          (unsigned long)(ulIPAddress & 0xffUL));
            pcTarget += rc;
            pcSource += 3; /* skip "<n>ip" */
        }
    }

    /* Add new line character */
    *pcTarget = '\r';
    pcTarget++;
    /* Add NULL character to mark the end of the string */
    *pcTarget = '\0';

    /* Print out into console */
    xil_printf(cOutputString);
}

/**************************************************************************//**
*  Routine:     vApplicationIPNetworkEventHook
*  @brief       This is the callback that is called by the TCP/IP stack when
*               the network either connects or disconnects.
*               The network event hook is a good place to create tasks that
*               use the IP stack as it ensures the tasks are not created until
*               the TCP/IP stack is ready.
*
*  @param       eNetworkEvent [in]  Contains if the network is connected or disconnected.
*
*  @return      None
******************************************************************************/
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    static BaseType_t   xTasksAlreadyCreated = pdFALSE;

    /* If the network has just come up...*/
    if(eNetworkEvent == eNetworkUp)
    {
        /* Create the tasks that use the TCP/IP stack if they have not already been created */
        if(xTasksAlreadyCreated == pdFALSE)
        {
            /* For convenience, tasks that use FreeRTOS+TCP can be created here
             * to ensure they are not created before the network is usable */
            // Resume the Ethernet Control Task


        	/* Create the two tasks.  The Tx task is given a lower priority than the
        	Rx task, so the Rx task will leave the Blocked state and pre-empt the Tx
        	task as soon as the Tx task places an item in the queue. */
        	xTaskCreate( 	tcpserver_task, 			/* The function that implements the task. */
        					( const char * ) "TcpServer", 	/* Text name for the task, provided to assist debugging only. */
        					1024, 	/* The stack allocated to the task. */
        					NULL, 						/* The task parameter is not used, so set to NULL. */
        					2,			/* The task runs at the priority 2. */
        					&xTask );

            xTasksAlreadyCreated = pdTRUE;
        }

        // Update network variable state
        e_network_state = NETWORK_UP;
    }
    /* If the network has just come down...*/
    else if(eNetworkEvent == eNetworkDown)
    {
        // Update network variable state
        e_network_state = NETWORK_DOWN;
    }
}

/**************************************************************************//**
*  Routine:     vApplicationPingReplyHook
*  @brief       This is the callback that is called by the TCP/IP stack when
*               a reply to an outgoing ping is received.
*
*  @param       eStatus [in]        The result of the ping reply
*  @param       usIdentifier [in]   Identifier field in the received ICMP message.
*
*  @return      None
******************************************************************************/
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
    switch(eStatus)
    {
        case eSuccess:
            FreeRTOS_debug_printf(("Ping reply received - identifier %d\n", usIdentifier));
            break;

        case eInvalidChecksum:
            FreeRTOS_debug_printf(("Ping reply received with invalid checksum - identifier %d\n", usIdentifier));
            break;

        case eInvalidData:
            FreeRTOS_debug_printf(("Ping reply received with invalid data - identifier %d\n", usIdentifier));
            break;

        default:
            break;
    }
}


/******************************************************************************
*
* Definition of Local Functions
*
******************************************************************************/
/**************************************************************************//**
*  Routine:     tcpserver_task
*  @brief       This function implements the WEB SERVER task
*
*  @param       pvParameters    [in] Pointer to parameters of task function
*
*  @return      None
******************************************************************************/
static void tcpserver_task(void *pvParameters)
{
	TCPServer_t	*px_tcp_server;
    uint32_t    u32_blocking_time;
    XStatus 	xStatus;

    xStatus = File_System_Init();
    configASSERT(xStatus == XST_SUCCESS);

    // Creates the TCP server defined by s_server_configuration variable
    px_tcp_server = FreeRTOS_CreateTCPServer(s_server_configuration,
                                        (sizeof(s_server_configuration)/sizeof(s_server_configuration[0])));

    u32_blocking_time = pdMS_TO_TICKS(10UL);

    // Infinite loop
    while(1)
    {
    	FreeRTOS_TCPServerWork(px_tcp_server, u32_blocking_time);
    }
}

static XStatus File_System_Init(void)
{
	static uint8_t ucRAMDisk[ mainRAM_DISK_SECTORS * mainRAM_DISK_SECTOR_SIZE ];

	/* Create the RAM disk. */
	pxRAMDisk = FF_RAMDiskInit( mainRAM_DISK_NAME, ucRAMDisk, mainRAM_DISK_SECTORS, mainIO_MANAGER_CACHE_SIZE );
	configASSERT( pxRAMDisk );
	if (pxRAMDisk == NULL)
	{
		xil_printf("Core#%d: RAM File system initialization failure.\r\n ", XPAR_CPU_ID);
		return XST_FAILURE;
	}

	/* Put Volume name*/
	Xil_MemCpy( pxRAMDisk->pxIOManager->xPartition.pcVolumeLabel, "CCU_PRO RAM", sizeof(pxRAMDisk->pxIOManager->xPartition.pcVolumeLabel)-1);

	/* Print out information on the RAM disk. */
	FF_RAMDiskShowPartition( pxRAMDisk );

	return XST_SUCCESS;
}
