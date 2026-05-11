/*
 * openswitch.c - OpenSYS Context Switch Manager
 *
 * Copyright (C) 2026 CazyUndee
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "process.h"
#include "scheduler.h"
#include "timer.h"
#include <stdint.h>

/* Assembly context switch */
extern void context_switch(uint64_t* old_ctx, uint64_t* new_ctx);

/* External scheduler functions */
extern process_t* process_current(void);
extern void scheduler_set_current(process_t* proc);
extern int scheduler_needs_reschedule(void);
extern void scheduler_clear_reschedule(void);
extern void process_check_sleepers(void);
extern process_t* scheduler_pick(void);
extern void scheduler_add(process_t* proc);
extern void scheduler_tick(void);

/* Main scheduler - called from timer interrupt */
void openswitch_schedule(void) {
    /* Check sleeping processes */
    process_check_sleepers();
    
    /* Get current process */
    process_t* current = process_current();
    
    /* Get next process to run */
    process_t* next = scheduler_pick();
    
    if (!next) {
        /* Nothing to run, stick with current */
        if (current) {
            scheduler_clear_reschedule();
        }
        return;
    }
    
    if (!current) {
        /* No current process, just run next */
        scheduler_set_current(next);
        scheduler_clear_reschedule();
        
        /* Jump to new process */
        context_switch(0, (uint64_t*)&next->context);
        return;
    }
    
    /* Save current and switch to next */
    if (current->state == PROC_STATE_RUNNING) {
        current->state = PROC_STATE_READY;
        scheduler_add(current);
    }
    
    scheduler_set_current(next);
    scheduler_clear_reschedule();
    
    /* Perform context switch */
    context_switch((uint64_t*)&current->context, (uint64_t*)&next->context);
}

/* Called from timer interrupt */
void openswitch_timer_tick(void) {
    scheduler_tick();
    
    if (scheduler_needs_reschedule()) {
        openswitch_schedule();
    }
}

// Legacy compatibility functions
void schedule(void) {
    openswitch_schedule();
}

void scheduler_timer_tick(void) {
    openswitch_timer_tick();
}
