#ifndef _TCPIP_API_INIT_H
#define _TCPIP_API_INIT_H



/******************************************************************************
 Includes
******************************************************************************/
/* Try not to use */


/******************************************************************************
 Constant and macro definitions
******************************************************************************/



/******************************************************************************
 Data Types
******************************************************************************/

/**
    @enum  ENetworkState_t
    @brief Network state options (equivalent to eIPCallbackEvent_t defined in FreeRTOS_IP.h)
*/
typedef enum
{
    NETWORK_UP,         //!< The network is configured and connected.
    NETWORK_DOWN        //!< The network connection has been lost.
} ENetworkState_t;


/******************************************************************************
 Public function definitions
******************************************************************************/

int init_xemacps(void (*p_start_net_apps)(void));
void start_network_applications(void);

ENetworkState_t get_network_state(void);


#endif  // _TCPIP_API_INIT_H
