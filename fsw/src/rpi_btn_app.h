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
**    Define the Raspberry Pi Button demo application
**
**  Notes:
**    1. This demo is intentionally kept simple so it can be used as a
**       starting point for soemone that wants to write an app for controlling
**       a Raspberry Pi interface using a hardware library like mipea. 
**
*/

#ifndef _rpi_btn_app_
#define _rpi_btn_app_

/*
** Includes
*/

#include "app_cfg.h"
#include "childmgr.h"
#include "initbl.h"
#include "button.h"

/***********************/
/** Macro Definitions **/
/***********************/

/*
** Events
*/

#define RPI_BTN_INIT_APP_EID    (RPI_BTN_BASE_EID + 0)
#define RPI_BTN_NOOP_EID        (RPI_BTN_BASE_EID + 1)
#define RPI_BTN_EXIT_EID        (RPI_BTN_BASE_EID + 2)
#define RPI_BTN_INVALID_MID_EID (RPI_BTN_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Command Packets
** - See EDS command definitions in rpi_btn.xml
*/


/******************************************************************************
** Telmetery Packets
** - See EDS command definitions in rpi_btn.xml
*/


/******************************************************************************
** RPI_BTN_Class
*/
typedef struct 
{

   /* 
   ** App Framework
   */ 
   
   INITBL_Class_t     IniTbl; 
   CFE_SB_PipeId_t    CmdPipe;
   CMDMGR_Class_t     CmdMgr;
   CHILDMGR_Class_t   ChildMgr;   
   
   /*
   ** Telemetry Packets
   */
   
   RPI_BTN_StatusTlm_t  StatusTlm;

   /*
   ** App State & Objects
   */ 
       
   uint32          PerfId;
   CFE_SB_MsgId_t  CmdMid;
   CFE_SB_MsgId_t  OneHzMid;
   
   BUTTON_Class_t  Button;
 
} RPI_BTN_Class_t;


/*******************/
/** Exported Data **/
/*******************/

extern RPI_BTN_Class_t  RpiBtn;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: RPI_BTN_AppMain
**
*/
void RPI_BTN_AppMain(void);


/******************************************************************************
** Function: RPI_BTN_NoOpCmd
**
*/
bool RPI_BTN_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: RPI_BTN_ResetAppCmd
**
*/
bool RPI_BTN_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


#endif /* _rpi_btn_app_ */
