#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

/*
 * Build selection for the exam project.
 *
 * Default: master board + UART required task.
 *
 * To flash the slave board, change BOARD_MASTER to BOARD_SLAVE.
 * To test the optional CAN task, change LINK_UART to LINK_CAN.
 *
 * Build the master and slave firmware separately; one binary cannot be both
 * roles at the same time because the GPIO/communication behavior differs.
 */
#if !defined(BOARD_MASTER) && !defined(BOARD_SLAVE)
#define BOARD_MASTER
#endif

#if !defined(LINK_UART) && !defined(LINK_CAN)
#define LINK_UART
#endif

#endif
