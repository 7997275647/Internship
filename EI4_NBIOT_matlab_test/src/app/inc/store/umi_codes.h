/*-----------------------------------------------------------------------------
Project configuration
-----------------------------------------------------------------------------*/
#include <os/config.h>

/*-----------------------------------------------------------------------------
System level includes
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Project level includes
-----------------------------------------------------------------------------*/
#include <os/store.h>

#define MAKE_UMI_CODE(b3, b2, b1, b0) \
    ((Umi_Code_t)(((b3) << 24) | ((b2) << 16) | ((b1) << 8) | (b0)))


#define UMI_CODE_MODEM_SIM_INFO \
    MAKE_UMI_CODE(200UL, 2UL, 64UL, 1UL)
#define UMI_CODE_MODEM_CFG \
    MAKE_UMI_CODE(200UL, 2UL, 64UL, 2UL)
#define UMI_CODE_MODEM_STATS \
    MAKE_UMI_CODE(200UL, 2UL, 64UL, 4UL)
#define UMI_CODE_MODEM_COMM_STATS \
    MAKE_UMI_CODE(200UL, 2UL, 64UL, 5UL)
#define UMI_CODE_MODEM_STATISTICS \
    MAKE_UMI_CODE(200UL, 2UL, 64UL, 6UL)
#define UMI_CODE_MODEM_EVENT_FIFO \
    MAKE_UMI_CODE(200UL, 2UL, 64UL, 7UL)
