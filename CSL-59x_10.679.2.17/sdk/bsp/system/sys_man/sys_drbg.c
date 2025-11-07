/**
 ****************************************************************************************
 *
 * @file sys_drbg.c
 *
 * @brief System deterministic random bit generator
 *
 * Copyright (C) 2021-2024 Renesas Electronics Corporation and/or its affiliates.
 * All rights reserved. Confidential Information.
 *
 * This software ("Software") is supplied by Renesas Electronics Corporation and/or its
 * affiliates ("Renesas"). Renesas grants you a personal, non-exclusive, non-transferable,
 * revocable, non-sub-licensable right and license to use the Software, solely if used in
 * or together with Renesas products. You may make copies of this Software, provided this
 * copyright notice and disclaimer ("Notice") is included in all such copies. Renesas
 * reserves the right to change or discontinue the Software at any time without notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS". RENESAS DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. TO THE
 * MAXIMUM EXTENT PERMITTED UNDER LAW, IN NO EVENT SHALL RENESAS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE, EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES. USE OF THIS SOFTWARE MAY BE SUBJECT TO TERMS AND CONDITIONS CONTAINED IN
 * AN ADDITIONAL AGREEMENT BETWEEN YOU AND RENESAS. IN CASE OF CONFLICT BETWEEN THE TERMS
 * OF THIS NOTICE AND ANY SUCH ADDITIONAL LICENSE AGREEMENT, THE TERMS OF THE AGREEMENT
 * SHALL TAKE PRECEDENCE. BY CONTINUING TO USE THIS SOFTWARE, YOU AGREE TO THE TERMS OF
 * THIS NOTICE.IF YOU DO NOT AGREE TO THESE TERMS, YOU ARE NOT PERMITTED TO USE THIS
 * SOFTWARE.
 *
 ****************************************************************************************
 */


#if dg_configUSE_SYS_DRBG

#include "sys_drbg.h"
#include "sdk_crc16.h"
#if dg_configUSE_SYS_TRNG
#include "sys_trng.h"
#include "sys_trng_internal.h"
#else
#include "hw_sys.h"
#endif /* dg_configUSE_SYS_TRNG */

#if dg_configUSE_CHACHA20_RAND
#include "chacha20.h"
#endif /* dg_configUSE_CHACHA20_RAND */

#if defined(OS_PRESENT)
#include "osal.h"
#include "interrupts.h"
#endif /* OS_PRESENT */

#include "sys_watchdog.h"

/*
 * MACROS
 *****************************************************************************************
 */

#if defined(OS_PRESENT)

#if defined(SYS_DRBG_MUTEX_CREATE) && defined(SYS_DRBG_MUTEX_GET) && defined(SYS_DRBG_MUTEX_PUT)
#else

__RETAINED static OS_MUTEX sys_drbg_mutex;

#define SYS_DRBG_MUTEX_CREATE()         do {                                                            \
                                                OS_ASSERT(sys_drbg_mutex == NULL);                      \
                                                OS_MUTEX_CREATE(sys_drbg_mutex);                        \
                                                OS_ASSERT(sys_drbg_mutex);                              \
                                        } while (0)
// The following macros check if sys_drbg_mutex is not NULL before OS_MUTEX_GET/PUT.
// In order to prevent software from running with interrupts disabled if scheduler
// has not been started.
#define SYS_DRBG_MUTEX_GET()            do {                                                            \
                                                if (sys_drbg_mutex) {                                   \
                                                        OS_MUTEX_GET(sys_drbg_mutex, OS_MUTEX_FOREVER); \
                                                }                                                       \
                                        } while (0)
#define SYS_DRBG_MUTEX_PUT()            do {                                                            \
                                                if (sys_drbg_mutex) {                                   \
                                                        OS_MUTEX_PUT(sys_drbg_mutex);                   \
                                                }                                                       \
                                        } while (0)
#endif /* SYS_DRBG_MUTEX_CREATE && SYS_DRBG_MUTEX_GET && SYS_DRBG_MUTEX_PUT */
#else
#define SYS_DRBG_MUTEX_CREATE()         do {} while (0)
#define SYS_DRBG_MUTEX_GET()            do {} while (0)
#define SYS_DRBG_MUTEX_PUT()            do {} while (0)
#endif /* OS_PRESENT */


#define SYS_DRBG_LOCK()                                 \
        do {                                            \
                SYS_DRBG_MUTEX_GET();                   \
        } while (0)

#define SYS_DRBG_UNLOCK()                               \
        do {                                            \
                SYS_DRBG_MUTEX_PUT();                   \
        } while (0)


#if defined(OS_PRESENT)
#define SYS_DRBG_PRIORITY                (OS_TASK_PRIORITY_LOWEST)
#endif /* OS_PRESENT */

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief sys drbg data structure
 */
typedef struct {
        uint32_t buffer[dg_configUSE_SYS_DRBG_BUFFER_LENGTH];   /**< the buffer which holds the random numbers */
        uint32_t threshold;                                     /**< the threshold level in the buffer */
        uint32_t index;                                         /**< the index in the buffer */
        uint8_t  request;                                       /**< the request for buffer update */
} sys_drbg_t;

/*
 * VARIABLE DECLARATIONS
 *****************************************************************************************
 */

#define SYS_DRBG_RETAINED_UNINIT        __RETAINED_UNINIT
#define SYS_DRBG_DATA_STRUCT            __RETAINED

SYS_DRBG_DATA_STRUCT static sys_drbg_t sys_drbg;

#if dg_configUSE_STDLIB_RAND
SYS_DRBG_RETAINED_UNINIT static unsigned rand_r_state;
#endif /* dg_configUSE_STDLIB_RAND */

#if defined(OS_PRESENT)
__RETAINED static OS_TASK sys_drbg_handle;
#endif /* OS_PRESENT */

#if (dg_configUSE_SYS_TRNG == 0)
#define SYS_DRBG_SEED_SIZE              (16)

SYS_DRBG_RETAINED_UNINIT static uint32_t drbg_id;
#endif /* dg_configUSE_SYS_TRNG */

/*
 * LOCAL FUNCTION DECLARATIONS
 *****************************************************************************************
 */

static const uint8_t *dg_get_seed(void);
static uint32_t dg_rand(void);

#if defined(OS_PRESENT)
static void sys_drbg_update(void);
static OS_TASK_FUNCTION(sys_drbg_task, pvParameters);
#endif /* OS_PRESENT */

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static const uint8_t *dg_get_seed(void)
{
#if dg_configUSE_SYS_TRNG
        return sys_trng_get_seed();
#else
        static uint32_t entropy_pool[SYS_DRBG_SEED_SIZE / sizeof(uint32_t)];
        const uint32_t *p = (uint32_t*)(MEMORY_CMAC_CACHERAM_END - SYS_DRBG_SEED_SIZE);
        hw_sys_enable_cmac_cache_ram();
        for (int i = 0; i < (SYS_DRBG_SEED_SIZE / sizeof(uint32_t)); i++) {
                entropy_pool[i] = *(p + i);
        }
        hw_sys_disable_cmac_cache_ram();
        const uint32_t offset = (uint32_t)entropy_pool;
        ASSERT_WARNING((offset & 0x00000003) == 0);
        return (uint8_t *) offset;
#endif
}

static uint32_t dg_rand(void)
{
#if dg_configUSE_CHACHA20_RAND
        return csprng_get_next_uint32();
#elif dg_configUSE_STDLIB_RAND
        return (uint32_t) rand_r(&rand_r_state);
#endif
}

#if defined(OS_PRESENT)
static OS_TASK_FUNCTION(sys_drbg_task, pvParameters)
{
        int8_t wdog_id;

        /* Register task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        while (1) {
                /* Notify watch dog on each loop since there's no other trigger for this */
                sys_watchdog_notify(wdog_id);

                /* Suspend monitoring while task is blocked on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                /* Wait to be notified for drbg buffer update request */
                OS_TASK_NOTIFY_TAKE(OS_TRUE, OS_TASK_NOTIFY_FOREVER);

                /* Resume watch dog monitoring */
                sys_watchdog_notify_and_resume(wdog_id);

                /* update DRBG */
                sys_drbg_update();
        }
}
#endif /* OS_PRESENT */

#if (dg_configUSE_SYS_TRNG == 0)
bool sys_drbg_can_run(void)
{
        if (drbg_id != (uint32_t) crc16_calculate(dg_get_seed(), SYS_DRBG_SEED_SIZE)) {
                return true;
        }
        return false;
}
#endif /* dg_configUSE_SYS_TRNG */

void sys_drbg_srand(void)
{
#if dg_configUSE_CHACHA20_RAND
        csprng_seed(dg_get_seed());
#elif dg_configUSE_STDLIB_RAND
        srand(* (const unsigned *) (dg_get_seed()));
#endif
}

#if defined(OS_PRESENT)
void sys_drbg_create_os_objects(void)
{
        /* Check scheduler's state */
        OS_ASSERT(OS_GET_TASK_SCHEDULER_STATE() != OS_SCHEDULER_NOT_STARTED);

        /* Create mutex. Called only once! */
        SYS_DRBG_MUTEX_CREATE();


        OS_BASE_TYPE status;

        /* Create the sys_drbg task */
        status = OS_TASK_CREATE("sys_drbg",                   /* The text name assigned to the task, for
                                                                 debug only; not used by the kernel. */
                                sys_drbg_task,                /* The function that implements the task. */
                                NULL,                         /* The parameter passed to the task. */
                                OS_MINIMAL_TASK_STACK_SIZE,
                                                              /* The number of bytes to allocate to the
                                                                 stack of the task. */
                                SYS_DRBG_PRIORITY,            /* The priority assigned to the task. */
                                sys_drbg_handle );            /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);
}
#endif /* OS_PRESENT */

void sys_drbg_init(void)
{
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        /* This operation could last long and could trigger watchdog time out.
         * This delay is bigger for QSPI Flash so watchdog is kicked to prevent it.
         */
        hw_watchdog_set_pos_val(dg_configWDOG_RESET_VALUE);
#endif /* dg_configCODE_LOCATION */

        /* Generate random numbers */
        for (int i = 0; i < ARRAY_LENGTH(sys_drbg.buffer); i++) {
                sys_drbg.buffer[i] = dg_rand();
        }

        sys_drbg.threshold = dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD;
        sys_drbg.index     = 0;
        sys_drbg.request   = 0;

}

SYS_DRBG_ERROR sys_drbg_read_rand(uint32_t *rand_number)
{
        SYS_DRBG_ERROR ret = SYS_DRBG_ERROR_NONE;

        SYS_DRBG_LOCK();

        if (sys_drbg.index < ARRAY_LENGTH(sys_drbg.buffer)) {
                /* Check if threshold has been reached or passed */
                if (sys_drbg.index >= sys_drbg.threshold ) {
                        sys_drbg.request = 1;
                }
                /* Get random number */
                *rand_number = sys_drbg.buffer[sys_drbg.index];
                sys_drbg.index++;
        } else {
                /* Buffer has been exhausted */
                sys_drbg.request = 1;
                ret = SYS_DRBG_ERROR_BUFFER_EXHAUSTED;
                /* Get one random number */
                *rand_number = dg_rand();
        }
#if defined(OS_PRESENT)
        if (sys_drbg.request == 1) {
                /* Notify the sys_drbg task */
                if (in_interrupt()) {
                        OS_TASK_NOTIFY_GIVE_FROM_ISR(sys_drbg_handle);
                } else {
                        OS_TASK_NOTIFY_GIVE(sys_drbg_handle);
                }
        }
#endif /* OS_PRESENT */

        SYS_DRBG_UNLOCK();

        return ret;
}

#if defined(OS_PRESENT)
static void sys_drbg_update(void)
#else
void sys_drbg_update(void)
#endif /* OS_PRESENT */
{
        SYS_DRBG_LOCK();

        if (sys_drbg.request == 1) {
                /* Update the consumed random numbers */
                for (int i = 0; i < sys_drbg.index; i++) {
                        sys_drbg.buffer[i] = dg_rand();
                }
                sys_drbg.index   = 0;
                sys_drbg.request = 0;
        }

        SYS_DRBG_UNLOCK();
}

uint32_t sys_drbg_read_index(void)
{
        SYS_DRBG_LOCK();

        uint32_t index = sys_drbg.index;

        SYS_DRBG_UNLOCK();

        return index;
}

uint32_t sys_drbg_read_threshold(void)
{
        SYS_DRBG_LOCK();

        uint32_t threshold = sys_drbg.threshold;

        SYS_DRBG_UNLOCK();

        return threshold;
}

uint8_t sys_drbg_read_request(void)
{
        SYS_DRBG_LOCK();

        uint8_t request = sys_drbg.request;

        SYS_DRBG_UNLOCK();

        return request;
}

#endif /* dg_configUSE_SYS_DRBG */


