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
**    Implement the RPI Button demo application
**
**  Notes:
**    1. See rpi_btn_app.h for details.
**
*/

/*
** Includes
*/

#include <string.h>
#include "rpi_btn_app.h"
#include "rpi_btn_eds_cc.h"

/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ    (&(RpiBtn.IniTbl))
#define  CMDMGR_OBJ    (&(RpiBtn.CmdMgr))
#define  CHILDMGR_OBJ  (&(RpiBtn.ChildMgr))
#define  BUTTON_OBJ    (&(RpiBtn.Button))


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static int32 InitApp(void);
static int32 ProcessCommands(void);
static void SendStatusTlm(void);


/**********************/
/** File Global Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in app_cfg.h
** Defines "static INILIB_CfgEnum IniCfgEnum"
*/
DEFINE_ENUM(Config,APP_CONFIG)  


static CFE_EVS_BinFilter_t  EventFilters[] =
{  
   /* Event ID                  Mask */
   {BUTTON_CHILD_TASK_EID,   CFE_EVS_FIRST_4_STOP}  // Use CFE_EVS_NO_FILTER to see all events

};


/*****************/
/** Global Data **/
/*****************/

RPI_BTN_Class_t  RpiBtn;


/******************************************************************************
** Function: RPI_BTN_AppMain
**
*/
void RPI_BTN_AppMain(void)
{

   uint32 RunStatus = CFE_ES_RunStatus_APP_ERROR;


   CFE_EVS_Register(EventFilters, sizeof(EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                    CFE_EVS_EventFilter_BINARY);

   if (InitApp() == CFE_SUCCESS) /* Performs initial CFE_ES_PerfLogEntry() call */
   {  
   
      RunStatus = CFE_ES_RunStatus_APP_RUN;
      
   }
   
   /*
   ** Main process loop
   */
   while (CFE_ES_RunLoop(&RunStatus))
   {

      RunStatus = ProcessCommands(); /* Pends indefinitely & manages CFE_ES_PerfLogEntry() calls */

   } /* End CFE_ES_RunLoop */

   CFE_ES_WriteToSysLog("RPI_BTN App terminating, err = 0x%08X\n", RunStatus);   /* Use SysLog, events may not be working */

   CFE_EVS_SendEvent(RPI_BTN_EXIT_EID, CFE_EVS_EventType_CRITICAL, "RPI_BTN App terminating, err = 0x%08X", RunStatus);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */

} /* End of RPI_BTN_AppMain() */


/******************************************************************************
** Function: RPI_BTN_NoOpCmd
**
*/

bool RPI_BTN_NoOpCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_SendEvent (RPI_BTN_NOOP_EID, CFE_EVS_EventType_INFORMATION,
                      "No operation command received for RPI_BTN App version %d.%d.%d",
                      RPI_BTN_MAJOR_VER, RPI_BTN_MINOR_VER, RPI_BTN_PLATFORM_REV);

   return true;


} /* End RPI_BTN_NoOpCmd() */


/******************************************************************************
** Function: RPI_BTN_ResetAppCmd
**
** Notes:
**   1. No need to pass an object reference to contained objects because they
**      already have a reference from when they were constructed
**
*/

bool RPI_BTN_ResetAppCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_ResetAllFilters();
   
   CMDMGR_ResetStatus(CMDMGR_OBJ);
   CHILDMGR_ResetStatus(CHILDMGR_OBJ);
   
   BUTTON_ResetStatus();
	  
   return true;

} /* End RPI_BTN_ResetAppCmd() */


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{

   int32 Status = APP_C_FW_CFS_ERROR;
   
   CHILDMGR_TaskInit_t ChildTaskInit;
   
   /*
   ** Initialize objects 
   */

   if (INITBL_Constructor(&RpiBtn.IniTbl, RPI_BTN_INI_FILENAME, &IniCfgEnum))
   {
   
      RpiBtn.PerfId    = INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_PERF_ID);
      RpiBtn.CmdMid    = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_RPI_BTN_CMD_TOPICID));
      RpiBtn.OneHzMid  = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_BC_SCH_1_HZ_TOPICID));
      
      CFE_ES_PerfLogEntry(RpiBtn.PerfId);

      /* Constructor sends error events */    
      ChildTaskInit.TaskName  = INITBL_GetStrConfig(INITBL_OBJ, CFG_CHILD_NAME);
      ChildTaskInit.PerfId    = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PERF_ID);
      ChildTaskInit.StackSize = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_STACK_SIZE);
      ChildTaskInit.Priority  = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PRIORITY);
      Status = CHILDMGR_Constructor(CHILDMGR_OBJ, 
                                    ChildMgr_TaskMainCallback,
                                    BUTTON_ChildTask, 
                                    &ChildTaskInit); 
  
   } /* End if INITBL Constructed */
  
   if (Status == CFE_SUCCESS)
   {

      BUTTON_Constructor(BUTTON_OBJ, &RpiBtn.IniTbl);

      /*
      ** Initialize app level interfaces
      */
      
      CFE_SB_CreatePipe(&RpiBtn.CmdPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_CMD_PIPE_DEPTH), INITBL_GetStrConfig(INITBL_OBJ, CFG_CMD_PIPE_NAME));  
      CFE_SB_Subscribe(RpiBtn.CmdMid,   RpiBtn.CmdPipe);
      CFE_SB_Subscribe(RpiBtn.OneHzMid, RpiBtn.CmdPipe);

      CMDMGR_Constructor(CMDMGR_OBJ);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,   NULL, RPI_BTN_NoOpCmd,     0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC,  NULL, RPI_BTN_ResetAppCmd, 0);

      CFE_MSG_Init(CFE_MSG_PTR(RpiBtn.StatusTlm.TelemetryHeader), CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_RPI_BTN_STATUS_TLM_TOPICID)), sizeof(RPI_BTN_StatusTlm_t));
   
      /*
      ** Application startup event message
      */
      CFE_EVS_SendEvent(RPI_BTN_INIT_APP_EID, CFE_EVS_EventType_INFORMATION,
                        "RPI_BTN App Initialized. Version %d.%d.%d",
                        RPI_BTN_MAJOR_VER, RPI_BTN_MINOR_VER, RPI_BTN_PLATFORM_REV);
                        
   } /* End if CHILDMGR constructed */
   
   return(Status);

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCommands
**
*/
static int32 ProcessCommands(void)
{

   int32  RetStatus = CFE_ES_RunStatus_APP_RUN;
   int32  SysStatus;

   CFE_SB_Buffer_t* SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;
   

   CFE_ES_PerfLogExit(RpiBtn.PerfId);
   SysStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, RpiBtn.CmdPipe, CFE_SB_PEND_FOREVER);
   CFE_ES_PerfLogEntry(RpiBtn.PerfId);

   if (SysStatus == CFE_SUCCESS)
   {
      
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);
   
      if (SysStatus == CFE_SUCCESS)
      {
  
         if (CFE_SB_MsgId_Equal(MsgId, RpiBtn.CmdMid)) 
         {
            
            CMDMGR_DispatchFunc(CMDMGR_OBJ, &SbBufPtr->Msg);
         
         } 
         else if (CFE_SB_MsgId_Equal(MsgId, RpiBtn.OneHzMid))
         {

            SendStatusTlm();
            
         }
         else
         {
            
            CFE_EVS_SendEvent(RPI_BTN_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid command packet, MID = 0x%04X",
                              CFE_SB_MsgIdToValue(MsgId));
         } 

      }
      else
      {
         
         CFE_EVS_SendEvent(RPI_BTN_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                           "CFE couldn't retrieve message ID from the message, Status = %d", SysStatus);
      }
      
   } /* Valid SB receive */ 
   else 
   {
   
         CFE_ES_WriteToSysLog("RPI_BTN software bus error. Status = 0x%08X\n", SysStatus);   /* Use SysLog, events may not be working */
         RetStatus = CFE_ES_RunStatus_APP_ERROR;
   }  
      
   return RetStatus;

} /* End ProcessCommands() */


/******************************************************************************
** Function: SendStatusTlm
**
*/
static void SendStatusTlm(void)
{
   
   RPI_BTN_StatusTlm_Payload_t *StatusTlmPayload = &RpiBtn.StatusTlm.Payload;
   
   StatusTlmPayload->ValidCmdCnt   = RpiBtn.CmdMgr.ValidCmdCnt;
   StatusTlmPayload->InvalidCmdCnt = RpiBtn.CmdMgr.InvalidCmdCnt;

   StatusTlmPayload->GpioConnected = RpiBtn.Button.GpioConnected;
   StatusTlmPayload->GpioPin       = RpiBtn.Button.GpioPin;
   StatusTlmPayload->PressedCount  = RpiBtn.Button.PressedCount;
   StatusTlmPayload->LastRead      = RpiBtn.Button.LastRead;
   StatusTlmPayload->LastWrite     = RpiBtn.Button.LastWrite;

   /*
   ** Button
   */ 
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(RpiBtn.StatusTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(RpiBtn.StatusTlm.TelemetryHeader), true);
   
} /* End SendStatusTlm() */
