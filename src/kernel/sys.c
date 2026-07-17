/*
 * syscall.c - System Call Implementation
 */

#include <stdint.h>
#include "syscall.h"
#include "process.h"
#include "scheduler.h"
#include "timer.h"
#include "ramfs.h"
#include "vm.h"
#include "pmm.h"
#include "vga.h"

extern void syscall_entry(void);

static void* syscall_table[] = {
    /* 0: exit */     0,
    /* 1: read */     0,
    /* 2: write */    0,
    /* 3: open */     0,
    /* 4: close */    0,
    /* 5: fork */     0,
    /* 6: exec */     0,
    /* 7: wait */     0,
    /* 8: yield */    0,
    /* 9: sleep */    0,
    /* 10: getpid */  0,
    /* 11: kill */    0,
    /* 12: brk */     0,
    /* 13: mmap */    0,
    /* 14: munmap */  0,
};

static int sys_exit(int code) {
    (void)code;
    process_t* proc = process_current();
    if (proc) {
        proc->state = PROC_STATE_ZOMBIE;
        scheduler_reschedule();
    }
    return 0;
}

static int sys_read(int fd, void* buf, uint32_t count) {
    if (fd < 0) return -1;
    int bytes = ramfs_read(fd, buf, count, 0);
    return bytes;
}

static int sys_write(int fd, const void* buf, uint32_t count) {
	if (fd == 1 || fd == 2) {
		const char* s = (const char*)buf;
		for (uint32_t i = 0; i < count; i++) {
			terminal_putchar(s[i]);
		}
		return count;
	}
	return -1;
}

static int sys_yield(void) {
    process_yield();
    return 0;
}

static int sys_sleep(uint64_t ms) {
    process_sleep(ms);
    return 0;
}

static int sys_getpid(void) {
    process_t* proc = process_current();
    return proc ? proc->pid : 0;
}

static int sys_open(const char* name) {
    int fd = ramfs_find(name);
    return fd;
}

static int sys_close(int fd) {
    (void)fd;
    return 0;
}

static int sys_fork(void) {
    process_t* parent = process_current();
    if (!parent) return -1;
    char child_name[PROCESS_NAME_LEN];
    int i;
    for (i = 0; i < PROCESS_NAME_LEN - 7 && parent->name[i]; i++)
        child_name[i] = parent->name[i];
    child_name[i] = '_'; child_name[i+1] = 'c'; child_name[i+2] = 'h';
    child_name[i+3] = 'i'; child_name[i+4] = 'l'; child_name[i+5] = 'd';
    child_name[i+6] = '\0';
    pid_t child_pid = process_create(child_name, 0, 0);
    if (!child_pid) return -1;
    process_t* child = process_get(child_pid);
    vm_space_t* child_vm = vm_clone_space(parent->vm);
    if (!child_vm) return -1;
    child->vm = child_vm;
    return child->pid;
}

static int sys_exec(const char* path) {
    (void)path;
    return -1;
}

static int sys_wait(int* status) {
    (void)status;
    process_yield();
    return -1;
}

static int sys_kill(pid_t pid) {
    process_t* proc = process_get(pid);
    if (!proc) return -1;
    proc->state = PROC_STATE_ZOMBIE;
    return 0;
}

static uint64_t sys_brk(uint64_t addr) {
    process_t* proc = process_current();
    if (!proc || !proc->vm) return -1;
    if (addr == 0) return proc->vm->heap_end;
    if (addr > proc->vm->heap_start) {
        proc->vm->heap_end = addr;
        return addr;
    }
    return -1;
}

static uint64_t sys_mmap(uint64_t addr, uint64_t size, uint64_t flags) {
    (void)addr;
    process_t* proc = process_current();
    if (!proc || !proc->vm || size == 0) return -1;
    uint64_t count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    void* virt = vm_alloc_pages(proc->vm, count, flags);
    return virt ? (uint64_t)virt : (uint64_t)-1;
}

static uint64_t sys_munmap(uint64_t addr, uint64_t size) {
    process_t* proc = process_current();
    if (!proc || !proc->vm || size == 0) return -1;
    uint64_t count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    vm_free_pages(proc->vm, (void*)addr, count);
    return 0;
}

uint64_t syscall_handler(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3) {
    switch (num) {
    case SYS_EXIT: return sys_exit(a1);
    case SYS_READ: return sys_read(a1, (void*)a2, a3);
    case SYS_WRITE: return sys_write(a1, (const void*)a2, a3);
    case SYS_OPEN: return sys_open((const char*)a1);
    case SYS_CLOSE: return sys_close(a1);
    case SYS_FORK: return sys_fork();
    case SYS_EXEC: return sys_exec((const char*)a1);
    case SYS_WAIT: return sys_wait((int*)a1);
    case SYS_YIELD: return sys_yield();
    case SYS_SLEEP: return sys_sleep(a1);
    case SYS_GETPID: return sys_getpid();
    case SYS_KILL: return sys_kill(a1);
    case SYS_BRK: return sys_brk(a1);
    case SYS_MMAP: return sys_mmap(a1, a2, a3);
    case SYS_MUNMAP: return sys_munmap(a1, a2);
    default: return -1;
    }
}

void syscall_init(void) {
    extern void idt_set_syscall_gate_wrapper(void);
    idt_set_syscall_gate_wrapper();
}
