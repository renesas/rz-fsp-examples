/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
 *
* SPDX-License-Identifier: BSD-3-Clause
*/
/**********************************************************************************************************************
 * File Name    :um_lwip_port_cfg.h
 * Version      :1
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History :
 *
 *
 *********************************************************************************************************************/

#ifndef UM_LWIP_CFG_H_
#define UM_LWIP_CFG_H_

/** Implements NULL checking of parameters between the Ethernet API and the driver.
 *  "1": Enabled "0": Disabled*/
#define UM_LWIP_PORT_CFG_PARAM_CHECKING_ENABLE         (1)
 /** Specify the task name for the Ethernet status monitoring task.*/
#define UM_LWIP_PORT_CFG_LAUNCHER_TASK_NAME            ("lwIP Port Launcher Task")
 /** Specify the priority of the Ethernet status monitoring task*/
#define UM_LWIP_PORT_CFG_LAUNCHER_TASK_PRIORITY        (3)
 /** Specifies the stack size for the Ethernet status monitoring task.*/
#define UM_LWIP_PORT_CFG_LAUNCHER_TASK_STACK_BTYE_SIZE (1024)
 /** Specifies the number of queues to store Ethernet link state notifications.*/
#define UM_LWIP_PORT_CFG_LAUNCHER_QUEUE_LENGTH         (16)

#endif /** UM_LWIP_CFG_H_ */



