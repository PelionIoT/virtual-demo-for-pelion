// ----------------------------------------------------------------------------
// Copyright 2016-2021 Pelion.
//
// SPDX-License-Identifier: Apache-2.0
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

#ifndef APPLICATION_MBEDTLS_USER_CONFIG_H
#define APPLICATION_MBEDTLS_USER_CONFIG_H

// Include base-configuration from client library
#include "mbedTLS/mbedTLSConfig_mbedOS.h"

// Enable new define to allow usage of Wi-SUN network security.
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_NIST_KW_C
#define MBEDTLS_SHA1_C

#include "mbedtls/check_config.h"

#endif /* APPLICATION_MBEDTLS_USER_CONFIG_H */
