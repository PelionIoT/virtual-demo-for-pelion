// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
//  
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include "ftcd_comm_base.h"


// check user target interface input
#if !defined(FCE_ETHERNET_INTERFACE) && !defined(FCE_SERIAL_INTERFACE)
#error Neither of FCE_SERIAL_INTERFACE or FCE_ETHERNET_INTERFACE is defined, please select the interface type you would like to use according to your target platform and compile again.
#endif


/**
* The communication socket port
* If FTD_SERVER_TCP_PORT defined as 0, random port will be generated.
*/
#ifdef __LINUX__
    #define FTD_SERVER_TCP_PORT 8888  // fixed port (virtual-demo)
#else
    #define FTD_SERVER_TCP_PORT 7777
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* Instantiate new factory communication object.
* The communication object may be through serial, mbed-os-socket or Linux native socket.
*/
FtcdCommBase *fce_create_comm_interface(void);

/**
* Releases any OS specific resources that were allocated in CreateCommInterface().
* This function may be left empty if there is nothing to destroy or evacuate.
*/
void fce_destroy_comm_interface(void);

#ifdef __cplusplus
}
#endif
