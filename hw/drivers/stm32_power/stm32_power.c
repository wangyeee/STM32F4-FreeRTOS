/* 
 * stm32_power.c
 * Implementation of STM32 power tracker.
 * RebbleOS
 *
 * Joshua Wise <joshua@joshuawise.com>
 *
 * The stm32_power power tracker is, conceptually, very simple: it takes
 * clock demands for each peripheral on the SoC, counts them, and then
 * serializes access to RCC registers.
 *
 * In the future, this power tracker could be used to provide
 * instrumentation for estimated battery consumption, and could be used to
 * track non-RCC platform peripherals (display, backlight, Bluetooth, flash
 * power management states, etcetera).
 *
 * This implementation violates the "OS depends on HAL, but HAL does not
 * depend on OS" theorem by using a FreeRTOS semaphore to enforce
 * serialization.  The HAL requires that each individual peripheral be
 * serialized externally (i.e., no locking is done for flash accesses inside
 * the HAL), but does not require that peripherals *as a whole* be
 * serialized; as a result, since this driver works on a multi-peripheral
 * scale, it enforces its own locking.  XXX: this is conceptually kind of
 * inconsistent, and we should either decide to do all of the locking inside
 * of the HAL, or we should decide to do all of the locking inside the OS
 * (and have a global lock for the HAL).
 *
 * For now, we use FreeRTOS critical sections, to make it permissible to
 * call into this API from an interrupt service routine.  Later, we may
 * switch back to a mutex, as necessary.
 *
 * XXX: Which STM32F4xx are Time series? STM32F446xx has what looks like
 * "0th-level clock gating" on AHB1 that we might be able to save a little
 * more power with.
 */

// #define STM32_POWER_USE_MUTEX

#if defined(STM32F4XX)
#    include "stm32f4xx.h"
#elif defined(STM32F2XX)
#    include "stm32f2xx.h"
#    include "stm32f2xx_rcc.h"
#    include "misc.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif

#include "stdio.h"
#include "stm32_power.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define MK_STORAGE(n, b) static uint8_t _power_state_##n[b] = {0};
STM32_POWER_EXPANDO(MK_STORAGE)
#undef MK_STORAGE

#ifdef STM32_POWER_USE_MUTEX
static StaticSemaphore_t stm32_power_mutex_mem;
static SemaphoreHandle_t stm32_power_mutex;
#endif

void stm32_power_init() {
#ifdef STM32_POWER_USE_MUTEX
    stm32_power_mutex = xSemaphoreCreateMutexStatic(&stm32_power_mutex_mem);
#endif
}

void stm32_power_incr(stm32_power_register_t reg, uint32_t domain, int incr) {
    int bits;
    uint8_t *statep;
    void (*clkcmd)(uint32_t periph, FunctionalState state);
    
#ifdef STM32_POWER_USE_MUTEX
    static BaseType_t xHigherPriorityTaskWoken;
    uint8_t interrupt_set = 0;

    if(is_interrupt_set())
    {
        xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreTakeFromISR(stm32_power_mutex, &xHigherPriorityTaskWoken);
        interrupt_set = 1;
    }
    else
    {
        xSemaphoreTake(stm32_power_mutex, portMAX_DELAY);
    }
#else
    uint32_t critical_state = taskENTER_CRITICAL_FROM_ISR();
#endif

    switch (reg) {
#define MK_CASE(n, b) case STM32_POWER_##n: statep = _power_state_##n; bits = b; clkcmd = RCC_##n##PeriphClockCmd; break;
    STM32_POWER_EXPANDO(MK_CASE)
#undef MK_CASE
    default:
        assert(!"bad reg for stm32_power_incr");
    }
    
    assert(incr == 1 || incr == -1);
    for (int i = 0; i < 32; i++) {
        if (!(domain & (1 << i)))
            continue;

        assert(i < bits && "stm32_power_incr had domain with bit too high for power register");
        assert(!((incr == 1) && (statep[i] == 0xFF)) && "stm32_power_incr overflow");
        assert(!((incr == -1) && (statep[i] == 0x0)) && "stm32_power_incr underflow");
        
        statep[i] += incr;
        
        clkcmd(1 << i, statep[i] ? ENABLE : DISABLE);
    }

//    clkcmd(domain, ENABLE);
#ifdef STM32_POWER_USE_MUTEX
    if (interrupt_set)
    {
        xSemaphoreGiveFromISR(stm32_power_mutex, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        xSemaphoreGive(stm32_power_mutex);
    }
#else
    taskEXIT_CRITICAL_FROM_ISR(critical_state);
#endif
}
