/*
 * scheduler.h - Process Scheduler
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "../include/process.h"

/* Initialize scheduler */
void scheduler_init(void);

/* Add process to ready queue */
void scheduler_add(process_t* proc);

/* Remove process from scheduler */
void scheduler_remove(process_t* proc);

/* Pick next process to run */
process_t* scheduler_pick(void);

/* Called by timer for time slice */
void scheduler_tick(void);

/* Force immediate reschedule */
void scheduler_reschedule(void);

/* Set current running process */
void scheduler_set_current(process_t* proc);

/* Check if reschedule is needed */
int scheduler_needs_reschedule(void);

/* Clear reschedule flag */
void scheduler_clear_reschedule(void);

/* Get number of runnable processes */
int scheduler_runnable_count(void);

/* Context switch (assembly) */
void context_switch(cpu_context_t* old_ctx, cpu_context_t* new_ctx);

#endif
