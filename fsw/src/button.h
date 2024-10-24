/*
**  Copyright 2022 bitValence, Inc.
**  All Rights Reserved.
**
**  This program is free software; you can modify and/or redistribute it
**  under the terms of the GNU Affero General Public License
**  as published by the Free Software Foundation; version 3 with
**  attribution addendums as found in the LICENSE.txt
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  Purpose:
**    Define Button class
**
**  Notes:
**    1. Code based on "Raspberry Pi And The IoT In C", https://iopress.info/index.php/books/raspberry-pi-iot-in-c
**       Chapter 8: Advanced Input – Events, Threads, Interrupts
**
*/

#ifndef _button_
#define _button_

/*
** Includes
*/
#include <poll.h>
#include "app_cfg.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define BUTTON_CONSTRUCTOR_EID  (BUTTON_BASE_EID + 0)
#define BUTTON_OPEN_GPIO_EID    (BUTTON_BASE_EID + 1)
#define BUTTON_SET_EDGE_EID     (BUTTON_BASE_EID + 2)
#define BUTTON_CHILD_TASK_EID   (BUTTON_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** BUTTON_Class
*/

typedef struct
{

   /*
   ** Framework References
   */
   
   INITBL_Class_t *IniTbl;


   /*
   ** Class State Data
   */
   struct pollfd PollFileDescr[1];
   
   bool  GpioConnected;
   uint8 GpioPin;
   
   int   FileDescr;
   int   PressedCount;
   int   LastRead;
   int   LastWrite;
   
} BUTTON_Class_t;



/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: BUTTON_Constructor
**
** Initialize the Button object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void BUTTON_Constructor(BUTTON_Class_t *ButtonPtr, INITBL_Class_t *IniTbl);


/******************************************************************************
** Function: BUTTON_ChildTask
**
*/
bool BUTTON_ChildTask(CHILDMGR_Class_t *ChildMgr);


/******************************************************************************
** Function: BUTTON_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in HK telemetry that doesn't
**      change the functional behavior should be reset.
**
*/
void BUTTON_ResetStatus(void);


#endif /* _button_ */
