/******************************************************************************
 * @file     drivers.h
 *
 * @brief    for TLSR chips
 *
 * @author   public@telink-semi.com;
 * @date     Sep. 30, 2010
 *
 * @attention
 *
 *  Copyright (C) 2019-2020 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *****************************************************************************/

#pragma once

#include "config.h"

#if(MCU_CORE_TYPE == MCU_CORE_825x)
    #include "drivers/8258/driver_8258.h"
    #include "drivers/8258/driver_ext/ext_rf.h"
    #include "drivers/8258/driver_ext/ext_pm.h"
    #include "drivers/8258/driver_ext/ext_misc.h"
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    #include "drivers/8278/driver_8278.h"
    #include "drivers/8278/driver_ext/ext_rf.h"
    #include "drivers/8278/driver_ext/ext_pm.h"
    #include "drivers/8278/driver_ext/ext_misc.h"
#else
    #error MCU core typr error !
#endif
