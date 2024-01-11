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
**    Define platform configurations for the RPI_Button demo application
**
**  Notes:
**    None
**
*/

#ifndef _rpi_btn_platform_cfg_
#define _rpi_btn_platform_cfg_

/*
** Includes
*/

#include "rpi_btn_mission_cfg.h"


/******************************************************************************
** Platform Deployment Configurations
*/

#define RPI_BTN_PLATFORM_REV   0
#define RPI_BTN_INI_FILENAME   "/cf/rpi_btn_ini.json"

#define RPI_BTN_DEV_STR_MAX  64  // Must accommodate the longest "BTN_DEV_STR_*" string defined in the ini file

#endif /* _rpi_btn_platform_cfg_ */
