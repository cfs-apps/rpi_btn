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
**    Implement the Button Class methods
**
**  Notes:
**    1. The Button obkject owner is responsible for calling the 
**       BUTTON_Constructor() prior to creating the child task using
**       BUTTON_ChildTask().
**    2. TODO: Look for RPI defines for hardcoded strings "both", "in" and "out"
**    3. TODO: Improve error handling logic
**
*/

/*
** Include Files:
*/

#include <string.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#include "button.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define DIR_IN_STR  "in"
#define DIR_OUT_STR "out"


/**********************/
/** Type Definitions **/
/**********************/

typedef enum
{

   DIR_IN  = 1,
   DIR_OUT = 2

} Direction_t;


/**********************/
/** Global File Data **/
/**********************/

static BUTTON_Class_t*  Button = NULL;


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static void ButtonPressed(void);
static bool OpenGpio(int GpioPin, Direction_t Direction);
static bool ReadGpio(void);
static bool SetGpioEdge(int GpioPin, const char *Edge);
//static bool WriteGpio(int Bit);


/******************************************************************************
** Function: BUTTON_Constructor
**
** Initialize the Button object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void BUTTON_Constructor(BUTTON_Class_t *ButtonPtr, INITBL_Class_t *IniTbl)
{
   
   Button = ButtonPtr;
   
   memset(Button, 0, sizeof(BUTTON_Class_t));
   Button->IniTbl  = IniTbl;
   Button->GpioPin = INITBL_GetIntConfig(Button->IniTbl, CFG_BTN_PIN);

   if (OpenGpio(Button->GpioPin, DIR_IN))
   {
      
      Button->PollFileDescr[0].fd      = Button->FileDescr;
      Button->PollFileDescr[0].events  = POLLPRI;
      Button->PollFileDescr[0].revents = 0;
      
      if (SetGpioEdge(Button->GpioPin, "both"))
      {
         ReadGpio();
         Button->GpioConnected = true;
         CFE_EVS_SendEvent(BUTTON_CONSTRUCTOR_EID, CFE_EVS_EventType_INFORMATION, 
                           "Sucessfully connected to GPIO pin %d", Button->GpioPin);
      }
   }
   
} /* End BUTTON_Constructor() */


/******************************************************************************
** Function: BUTTON_ChildTask
**
** Notes:
**   1. Returning false causes the child task to terminate.
**   2. Information events are sent because this is instructional code and the
**      events provide feedback. The events are filtered so they won't flood
**      the ground. A reset app command resets the event filter.  
**
*/
bool BUTTON_ChildTask(CHILDMGR_Class_t* ChildMgr)
{
   
   bool RetStatus = false;
   
   if (Button->GpioConnected)
   {
      int Status = poll(Button->PollFileDescr, 1, -1);
      if ((Status > 0) && (Button->PollFileDescr[0].revents & POLLPRI))
      {
         ButtonPressed();
         lseek(Button->PollFileDescr[0].fd, 0, SEEK_SET);
         ReadGpio();
         CFE_EVS_SendEvent(BUTTON_CHILD_TASK_EID, CFE_EVS_EventType_INFORMATION, 
                           "Button pressed, read file value %d", Button->LastRead);
      }
      
      OS_TaskDelay(50);
      RetStatus = true;
   
   } /* End if connected */
   
   return RetStatus;


} /* End BUTTON_ChildTask() */


/******************************************************************************
** Function: BUTTON_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in status telemetry that
**      doesn't change the functional behavior should be reset.
**
*/
void BUTTON_ResetStatus(void)
{

   return;

} /* End BUTTON_ResetStatus() */


/******************************************************************************
**
** Process a button pressed event
**
*/
static void ButtonPressed(void)
{
   
   Button->PressedCount++;
   
} /* End ButtonPressed() */


/******************************************************************************
**
** Attach an interrupt handler to a GPIO pin
**
*/
bool OpenGpio(int GpioPin, Direction_t Direction)
{

   int StrLen;
   char StrBuf[RPI_BTN_DEV_STR_MAX];

   if (GpioPin < 0 || GpioPin > 31)
   {
      CFE_EVS_SendEvent (BUTTON_OPEN_GPIO_EID, CFE_EVS_EventType_ERROR, 
                         "Invalid GPIO pin %d. Value must be in range [0..31]", GpioPin);      
      return false;
   }
   
   if (Direction != DIR_IN && Direction != DIR_OUT)
   {
      CFE_EVS_SendEvent (BUTTON_OPEN_GPIO_EID, CFE_EVS_EventType_ERROR, 
                         "Invalid direction %d. Value must be %d(in) or %d(out)", Direction, DIR_IN, DIR_OUT);      
      return false;
   }
   
   // GPIO pin string used for EXPORT operations 
   StrLen = snprintf(StrBuf, RPI_BTN_DEV_STR_MAX, "%d", GpioPin);
   if (Button->FileDescr != 0)
   {
      close(Button->FileDescr);
      Button->FileDescr = open(INITBL_GetStrConfig(Button->IniTbl, CFG_BTN_DEV_STR_UNEXPORT), O_WRONLY);
      write(Button->FileDescr, StrBuf, StrLen);
      close(Button->FileDescr);
      Button->FileDescr = 0;
   }
   
   Button->FileDescr = open(INITBL_GetStrConfig(Button->IniTbl, CFG_BTN_DEV_STR_EXPORT), O_WRONLY);
   write(Button->FileDescr, StrBuf, StrLen);
   close(Button->FileDescr);
   StrLen = snprintf(StrBuf, RPI_BTN_DEV_STR_MAX,INITBL_GetStrConfig(Button->IniTbl, CFG_BTN_DEV_STR_DIRECTION), GpioPin);
   Button->FileDescr = open(StrBuf, O_WRONLY);
   
   if (Direction == DIR_OUT)
   {
      write(Button->FileDescr, DIR_OUT_STR, strlen(DIR_OUT_STR)+1);
      close(Button->FileDescr);
      StrLen = snprintf(StrBuf, RPI_BTN_DEV_STR_MAX, INITBL_GetStrConfig(Button->IniTbl, CFG_BTN_DEV_STR_VALUE), GpioPin);
      Button->FileDescr = open(StrBuf, O_WRONLY);
   } 
   else
   {
       write(Button->FileDescr, DIR_IN_STR, strlen(DIR_IN_STR)+1);
       close(Button->FileDescr);
       StrLen = snprintf(StrBuf, RPI_BTN_DEV_STR_MAX, INITBL_GetStrConfig(Button->IniTbl, CFG_BTN_DEV_STR_VALUE), GpioPin);
       Button->FileDescr = open(StrBuf, O_RDONLY);
   }
   
   return true;
 
} /* OpenGpio() */


/******************************************************************************
**
** Read from a GPIO pin
**
*/
static bool ReadGpio(void)
{

   char ValueStr[3];
   int  Status = read(Button->FileDescr, ValueStr, 3);
 
   if (Status > 0)
   {
      lseek(Button->FileDescr, 0, SEEK_SET);
      if (ValueStr[0] == '0')
      {
         Button->LastRead = 0;
      }
      else
      {
         Button->LastRead = 1;
      }
   }
   
   return true;
   
} /* ReadGpio() */


/******************************************************************************
**
** Set which edge will trigger interrupt: rising, falling, both
**
*/
static bool SetGpioEdge(int GpioPin, const char *Edge) 
{

   char StrBuf[RPI_BTN_DEV_STR_MAX];
   int FileDescr;
   bool RetStatus = false;
   
   snprintf(StrBuf, RPI_BTN_DEV_STR_MAX, INITBL_GetStrConfig(Button->IniTbl, CFG_BTN_DEV_STR_EDGE), GpioPin);
   FileDescr = open(StrBuf, O_WRONLY);
   if (FileDescr >= 0)
   {
      write(FileDescr, Edge, strlen(Edge)+1);
      close(FileDescr);
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent (BUTTON_SET_EDGE_EID, CFE_EVS_EventType_ERROR, 
                         "Failed to set edge %s to %s", StrBuf, Edge);         
   }
   
   return RetStatus;

} /* End SetGpioEdge() */


/******************************************************************************
**
** Write to button GPIO pin
**
*/
/*
static bool WriteGpio(int Bit)
{
   
   if (Bit == 0) 
   {
      write(Button->FileDescr, "0", 1);
      Button->LastWrite = 0;
   }
   else 
   {
      write(Button->FileDescr, "1", 1);
      Button->LastWrite = 1;
   }
   
   lseek(Button->FileDescr, 0, SEEK_SET);
 
   return true;

} WriteGpio() */