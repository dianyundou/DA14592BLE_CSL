/**
 * \file mbedtls_config.h
 *
 * \brief Configuration for Mbed TLS
 */

/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Copyright (C) 2024 Modified by Renesas Electronics Corporation
 */

#ifndef MBEDTLS_CONFIG_H_
#define MBEDTLS_CONFIG_H_

#include "mbedtls_port.h"

/* System support */
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_CALLOC_MACRO   mbedtls_port_calloc
#define MBEDTLS_PLATFORM_FREE_MACRO     mbedtls_port_free
#define MBEDTLS_DEPRECATED_WARNING

/* Mbed Crypto modules */
#define MBEDTLS_AES_C
#define MBEDTLS_MD_C
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C

#endif /* MBEDTLS_CONFIG_H_ */
