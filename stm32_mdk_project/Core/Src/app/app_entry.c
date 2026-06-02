#include "app.h"
#include "app_config.h"
#include "stm32f1xx_hal.h"

/*
 * Optional compile-time dispatcher.
 *
 * Define exactly one board role:
 *   BOARD_MASTER or BOARD_SLAVE
 *
 * Define exactly one communication link:
 *   LINK_UART or LINK_CAN
 *
 * Then call app_init() once and app_task(HAL_GetTick()) in the main loop.
 */

#if defined(BOARD_MASTER) == defined(BOARD_SLAVE)
#error "Define exactly one of BOARD_MASTER or BOARD_SLAVE"
#endif

#if defined(LINK_UART) == defined(LINK_CAN)
#error "Define exactly one of LINK_UART or LINK_CAN"
#endif

#if defined(BOARD_MASTER) && defined(LINK_UART)
void app_master_uart_init(void);
void app_master_uart_task(uint32_t now_ms);
#define APP_SELECTED_INIT app_master_uart_init
#define APP_SELECTED_TASK app_master_uart_task
#elif defined(BOARD_SLAVE) && defined(LINK_UART)
void app_slave_uart_init(void);
void app_slave_uart_task(uint32_t now_ms);
#define APP_SELECTED_INIT app_slave_uart_init
#define APP_SELECTED_TASK app_slave_uart_task
#elif defined(BOARD_MASTER) && defined(LINK_CAN)
void app_master_can_init(void);
void app_master_can_task(uint32_t now_ms);
#define APP_SELECTED_INIT app_master_can_init
#define APP_SELECTED_TASK app_master_can_task
#elif defined(BOARD_SLAVE) && defined(LINK_CAN)
void app_slave_can_init(void);
void app_slave_can_task(uint32_t now_ms);
#define APP_SELECTED_INIT app_slave_can_init
#define APP_SELECTED_TASK app_slave_can_task
#endif

void app_init(void)
{
    APP_SELECTED_INIT();
}

void app_task(uint32_t now_ms)
{
    APP_SELECTED_TASK(now_ms);
}
