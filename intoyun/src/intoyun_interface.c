/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/
#include "intoyun_interface.h"

const intoyun_t IntoYun =
{
    intoyunInit,
    intoyunSetEventCallback,
    intoyunSetMode,
    intoyunLoop,
    intoyunDatapointControl,
    intoyunSendProductInfo,

    intoyunDefineDatapointBool,
    intoyunDefineDatapointNumber,
    intoyunDefineDatapointEnum,
    intoyunDefineDatapointString,
    intoyunDefineDatapointBinary,

    intoyunReadDatapointBool,
    intoyunReadDatapointNumberInt32,
    intoyunReadDatapointNumberDouble,
    intoyunReadDatapointEnum,
    intoyunReadDatapointString,
    intoyunReadDatapointBinary,

    intoyunWriteDatapointBool,
    intoyunWriteDatapointNumberInt32,
    intoyunWriteDatapointNumberDouble,
    intoyunWriteDatapointEnum,
    intoyunWriteDatapointString,
    intoyunWriteDatapointBinary,

    intoyunSendDatapointBool,
    intoyunSendDatapointNumberInt32,
    intoyunSendDatapointNumberDouble,
    intoyunSendDatapointEnum,
    intoyunSendDatapointString,
    intoyunSendDatapointBinary,

    intoyunSendAllDatapointManual,
    intoyunSendCustomData,
};
