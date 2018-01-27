/* tick_timer_service.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include "librebble.h"
#include "appmanager.h"

typedef struct TickTimerState {
    CoreTimer timer; /* must be at the start of the struct! */
    int onqueue;
    TimeUnits units;
    TickHandler handler;
    struct tm lasttm;
} TickTimerState;

/* XXX: this should probably be per-app.  oh, well */
static TickTimerState _state = {.onqueue = 0};

static void _tick_timer_callback(CoreTimer *timer);

/* Updates the timer interval to fire for the next smallest requested unit,
 * then add to the timer queue.  */
static void _tick_timer_update_next(TickTimerState *state)
{
    if (state->onqueue) {
        appmanager_timer_remove(&state->timer);
    }
    
    /* Figure out the desired time. */
    time_t dtime;
    struct tm nexttm;
    
    dtime = rcore_mktime(&state->lasttm);

    if (state->units & SECOND_UNIT) {
        dtime = dtime + 1;
    } else if (state->units & MINUTE_UNIT) {
        dtime = dtime + 60;

        rcore_localtime(&nexttm, dtime);
        nexttm.tm_sec = 0;
        dtime = rcore_mktime(&nexttm);
    } else if (state->units & (HOUR_UNIT | DAY_UNIT | MONTH_UNIT | YEAR_UNIT)) {
        /* Everyone else gets woken up hourly, and we'll just cancel it
         * later if it wasn't requested.  */
        dtime = dtime + 60 * 60;
        
        rcore_localtime(&nexttm, dtime);
        nexttm.tm_min = nexttm.tm_sec = 0;
        dtime = rcore_mktime(&nexttm);
    }
    
    state->timer.when = rcore_time_to_ticks(dtime, 0);
    state->timer.callback = _tick_timer_callback;
    appmanager_timer_add(&state->timer);
    state->onqueue = 1;
}

static void _tick_timer_callback(CoreTimer *timer)
{
    TickTimerState *state = (TickTimerState *) timer;
    
    time_t time;
    struct tm tm;
    
    state->onqueue = 0;
    
    rcore_time_ms(&time, NULL);
    rcore_localtime(&tm, time);
    
    TimeUnits units = 0;
    /* XXX: Does a real pebbleos return a bitmask, or only the MSB? */
    if (tm.tm_sec != state->lasttm.tm_sec) units |= SECOND_UNIT;
    if (tm.tm_min != state->lasttm.tm_min) units |= MINUTE_UNIT;
    if (tm.tm_hour != state->lasttm.tm_hour) units |= HOUR_UNIT;
    if (tm.tm_mday != state->lasttm.tm_mday) units |= DAY_UNIT;
    if (tm.tm_mon != state->lasttm.tm_mon) units |= MONTH_UNIT;
    if (tm.tm_year != state->lasttm.tm_year) units |= YEAR_UNIT;

    /* Update before we call in -- otherwise, they could unsubscribe, and
     * we'd just blissfully readd ourselves to the queue.  */
    memcpy(&state->lasttm, &tm, sizeof(tm));
    _tick_timer_update_next(state);
    
    if (units & state->units)
        state->handler(&tm, units);
}

void tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler)
{
    TickTimerState *state = &_state;

    time_t time;
    
    rcore_time_ms(&time, NULL);
    rcore_localtime(&state->lasttm, time);
    
    state->units = tick_units;
    state->handler = handler;
    
    _tick_timer_update_next(state);
}

void tick_timer_service_unsubscribe(void)
{
    TickTimerState *state = &_state;
    
    if (state->onqueue) {
        appmanager_timer_remove(&state->timer);
        state->onqueue = 0;
    }
}


