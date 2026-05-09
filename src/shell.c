/*
 * shell.c - Natural Language Shell
 *
 * Commands are natural phrases instead of cryptic abbreviations.
 */

#include <stdint.h>
#include <stddef.h>
#include "../include/vga.h"
#include "../include/ps2_keyboard.h"
#include "../include/fs.h"
#include "../include/process.h"
#include "../include/scheduler.h"
#include "../include/pmm.h"
#include "../include/rtc.h"

#define MAX_CMD_LEN 256

static char cwd[256] = "/";

static int k_strlen(const char* s) { int n = 0; while (*s++) n++; return n; }
static int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *a - *b;
}
static int k_strncmp(const char* a, const char* b, int n) {
    while (n-- && *a && *a == *b) { a++; b++; }
    return n < 0 ? 0 : *a - *b;
}
static char* trim(char* s) {
    while (*s == ' ') s++;
    char* end = s + k_strlen(s) - 1;
    while (end > s && *end == ' ') *end-- = 0;
    return s;
}

static int cmd_equals(const char* input, const char* pattern) {
    return k_strcmp(input, pattern) == 0;
}

static void list_callback(const char* name, int is_dir, uint32_t size) {
    terminal_writestring("  ");
    if (is_dir) {
        terminal_writestring("[DIR]  ");
    } else {
        terminal_writestring("  ");
        terminal_put_dec(size);
        terminal_writestring(" bytes  ");
    }
    terminal_writestring_nl(name);
}

static void cmd_list(void) {
    terminal_writestring_nl("");
    int count = fs_readdir("/", list_callback);
    if (count == 0) {
        terminal_writestring_nl("  (empty filesystem)");
    } else {
        terminal_writestring_nl("");
    }
}

static void cmd_show_memory(void) {
    uint64_t total = pmm_get_total() / (1024 * 1024);
    uint64_t free = pmm_get_free() / (1024 * 1024);

    terminal_writestring_nl("");
    terminal_writestring("  Total RAM: ");
    terminal_put_dec(total);
    terminal_writestring_nl(" MB");
    terminal_writestring("  Free RAM:  ");
    terminal_put_dec(free);
    terminal_writestring_nl(" MB");
    terminal_writestring_nl("");
}

static void cmd_ps(void) {
    terminal_writestring_nl("");
    terminal_writestring_nl("  PID  Name        State     CPU Time");
    terminal_writestring_nl("  ---  ----------  --------  --------");

    extern process_t* process_get_by_index(int i);
    extern int process_get_count(void);

    int count = 0;
    for (int i = 0; i < 64; i++) {
        process_t* p = process_get_by_index(i);
        if (p && p->state != 0) {
            terminal_writestring("  ");
            terminal_put_dec(p->pid);
            terminal_writestring("  ");
            terminal_writestring(p->name);

            int namelen = k_strlen(p->name);
            for (int j = namelen; j < 10; j++) terminal_putchar(' ');

            const char* state_str = "???";
            if (p->state == 1) state_str = "READY";
            else if (p->state == 2) state_str = "RUNNING";
            else if (p->state == 3) state_str = "BLOCKED";
            else if (p->state == 4) state_str = "ZOMBIE";
            terminal_writestring(state_str);

            terminal_writestring("  ");
            terminal_put_dec(p->cpu_time);
            terminal_writestring_nl("ms");
            count++;
        }
    }

    terminal_writestring("\n  ");
    terminal_put_dec(count);
    terminal_writestring_nl(" processes total");
    terminal_writestring_nl("");
}

static void cmd_date(void) {
    rtc_time_t t;
    rtc_read_time(&t);

    terminal_writestring_nl("");
    terminal_writestring("  ");

    terminal_put_dec(t.month);
    terminal_putchar('/');
    terminal_put_dec(t.day);
    terminal_putchar('/');
    terminal_put_dec(t.century);
    terminal_put_dec(t.year);

    terminal_writestring("  ");

    if (t.hour < 10) terminal_putchar('0');
    terminal_put_dec(t.hour);
    terminal_putchar(':');
    if (t.minute < 10) terminal_putchar('0');
    terminal_put_dec(t.minute);
    terminal_putchar(':');
    if (t.second < 10) terminal_putchar('0');
    terminal_put_dec(t.second);

    terminal_writestring_nl("");
    terminal_writestring_nl("");
}

static void cmd_create_file(const char* name) {
    if (!name || !*name) {
        terminal_writestring_nl("  Usage: create <filename>");
        return;
    }
    char path[256];
    if (name[0] == '/') {
        int len = 0;
        while (name[len] && len < 255) {
            path[len] = name[len];
            len++;
        }
        path[len] = 0;
    } else {
        path[0] = '/';
        int len = 1;
        while (name[len-1] && len < 255) {
            path[len] = name[len-1];
            len++;
        }
        path[len] = 0;
    }
    
    fs_file_t* file = fs_open(path, 1);
    if (!file) {
        terminal_writestring_nl("  Error: Could not create file");
    } else {
        fs_close(file);
        terminal_writestring("  Created: ");
        terminal_writestring_nl(name);
    }
}

static void cmd_mkdir(const char* name) {
    if (!name || !*name) {
        terminal_writestring_nl("  Usage: mkdir <dirname>");
        return;
    }
    char path[256];
    if (name[0] == '/') {
        int len = 0;
        while (name[len] && len < 255) {
            path[len] = name[len];
            len++;
        }
        path[len] = 0;
    } else {
        path[0] = '/';
        int len = 1;
        while (name[len-1] && len < 255) {
            path[len] = name[len-1];
            len++;
        }
        path[len] = 0;
    }
    
    if (fs_mkdir(path) < 0) {
        terminal_writestring_nl("  Error: Could not create directory");
    } else {
        terminal_writestring("  Created directory: ");
        terminal_writestring_nl(name);
    }
}

static void cmd_delete(const char* name) {
    if (!name || !*name) {
        terminal_writestring_nl("  Usage: delete <filename>");
        return;
    }
    char path[256];
    if (name[0] == '/') {
        int len = 0;
        while (name[len] && len < 255) {
            path[len] = name[len];
            len++;
        }
        path[len] = 0;
    } else {
        path[0] = '/';
        int len = 1;
        while (name[len-1] && len < 255) {
            path[len] = name[len-1];
            len++;
        }
        path[len] = 0;
    }
    
    if (fs_unlink(path) < 0) {
        terminal_writestring_nl("  Error: File not found");
    } else {
        terminal_writestring("  Deleted: ");
        terminal_writestring_nl(name);
    }
}

static void cmd_write_file(const char* name, const char* content) {
    if (!name || !*name) {
        terminal_writestring_nl("  Usage: write <filename> <content>");
        return;
    }
    char path[256];
    if (name[0] == '/') {
        int len = 0;
        while (name[len] && len < 255) {
            path[len] = name[len];
            len++;
        }
        path[len] = 0;
    } else {
        path[0] = '/';
        int len = 1;
        while (name[len-1] && len < 255) {
            path[len] = name[len-1];
            len++;
        }
        path[len] = 0;
    }
    
    fs_file_t* file = fs_open(path, 1);
    if (!file) {
        terminal_writestring_nl("  Error: Could not open file for writing");
        return;
    }
    int len = k_strlen(content);
    size_t written = fs_write(file, content, len);
    fs_close(file);
    terminal_writestring("  Wrote ");
    terminal_put_dec(written);
    terminal_writestring(" bytes to ");
    terminal_writestring_nl(name);
}

static void cmd_read_file(const char* name) {
    if (!name || !*name) {
        terminal_writestring_nl("  Usage: read <filename>");
        return;
    }
    char path[256];
    if (name[0] == '/') {
        int len = 0;
        while (name[len] && len < 255) {
            path[len] = name[len];
            len++;
        }
        path[len] = 0;
    } else {
        path[0] = '/';
        int len = 1;
        while (name[len-1] && len < 255) {
            path[len] = name[len-1];
            len++;
        }
        path[len] = 0;
    }
    
    fs_file_t* file = fs_open(path, 0);
    if (!file) {
        terminal_writestring_nl("  Error: File not found");
        return;
    }
    
    if (file->size == 0) {
        terminal_writestring_nl("  (empty file)");
        fs_close(file);
        return;
    }

    char buf[256];
    size_t to_read = file->size > 255 ? 255 : file->size;
    size_t read = fs_read(file, buf, to_read);
    buf[read] = 0;
    fs_close(file);

    terminal_writestring_nl("");
    terminal_writestring_nl(buf);
    terminal_writestring_nl("");
}

static void cmd_file_info(const char* name) {
    if (!name || !*name) {
        terminal_writestring_nl("  Usage: info <filename>");
        return;
    }
    char path[256];
    if (name[0] == '/') {
        int len = 0;
        while (name[len] && len < 255) {
            path[len] = name[len];
            len++;
        }
        path[len] = 0;
    } else {
        path[0] = '/';
        int len = 1;
        while (name[len-1] && len < 255) {
            path[len] = name[len-1];
            len++;
        }
        path[len] = 0;
    }
    
    fs_file_t* file = fs_open(path, 0);
    if (!file) {
        terminal_writestring_nl("  Error: File not found");
        return;
    }

    terminal_writestring_nl("");
    terminal_writestring("  Name:   ");
    terminal_writestring_nl(name);
    terminal_writestring("  Size:   ");
    terminal_put_dec(file->size);
    terminal_writestring_nl(" bytes");
    terminal_writestring("  Type:   File");
    terminal_writestring_nl("");
    
    fs_close(file);
}

static void show_help(void) {
    terminal_writestring_nl("");
    terminal_writestring_nl("  Natural Language Commands:");
    terminal_writestring_nl("  -------------------------");
    terminal_writestring_nl("  list              - show all files");
    terminal_writestring_nl("  create <n>        - create new file");
    terminal_writestring_nl("  mkdir <n>         - create directory");
    terminal_writestring_nl("  delete <n>        - delete file/dir");
    terminal_writestring_nl("  write <n> <text>  - write to file");
    terminal_writestring_nl("  read <n>          - display file contents");
    terminal_writestring_nl("  info <n>          - show file information");
    terminal_writestring_nl("  ps                - list processes");
    terminal_writestring_nl("  memory            - show memory usage");
    terminal_writestring_nl("  date              - show current date/time");
    terminal_writestring_nl("  clear             - clear screen");
    terminal_writestring_nl("  help              - show this help");
    terminal_writestring_nl("");
}

static void process_command(char* cmd) {
    cmd = trim(cmd);
    if (k_strlen(cmd) == 0) return;

    char* arg1 = cmd;
    while (*arg1 && *arg1 != ' ') arg1++;
    if (*arg1 == ' ') {
        *arg1 = 0;
        arg1++;
        while (*arg1 == ' ') arg1++;
    }

    char* arg2 = arg1;
    while (*arg2 && *arg2 != ' ') arg2++;
    if (*arg2 == ' ') {
        *arg2 = 0;
        arg2++;
        while (*arg2 == ' ') arg2++;
    }

    if (cmd_equals(cmd, "list") || cmd_equals(cmd, "ls") || cmd_equals(cmd, "dir")) {
        cmd_list();
    }
    else if (cmd_equals(cmd, "ps") || cmd_equals(cmd, "procs") || cmd_equals(cmd, "processes")) {
        cmd_ps();
    }
    else if (cmd_equals(cmd, "date") || cmd_equals(cmd, "time")) {
        cmd_date();
    }
    else if (cmd_equals(cmd, "create") || cmd_equals(cmd, "touch") || cmd_equals(cmd, "make")) {
        cmd_create_file(arg1);
    }
    else if (cmd_equals(cmd, "mkdir")) {
        cmd_mkdir(arg1);
    }
    else if (cmd_equals(cmd, "delete") || cmd_equals(cmd, "rm") || cmd_equals(cmd, "del")) {
        cmd_delete(arg1);
    }
    else if (cmd_equals(cmd, "write")) {
        cmd_write_file(arg1, arg2);
    }
    else if (cmd_equals(cmd, "read") || cmd_equals(cmd, "cat") || cmd_equals(cmd, "type")) {
        cmd_read_file(arg1);
    }
    else if (cmd_equals(cmd, "info")) {
        cmd_file_info(arg1);
    }
    else if (cmd_equals(cmd, "memory") || cmd_equals(cmd, "mem")) {
        cmd_show_memory();
    }
    else if (cmd_equals(cmd, "clear") || cmd_equals(cmd, "cls")) {
        terminal_clear();
    }
    else if (cmd_equals(cmd, "help") || cmd_equals(cmd, "?")) {
        show_help();
    }
    else {
        terminal_writestring("  Unknown command: \"");
        terminal_writestring(cmd);
        terminal_writestring_nl("\"");
        terminal_writestring_nl("  Type 'help' for available commands.");
    }
}

void shell_run(void) {
    char cmd_buffer[MAX_CMD_LEN];
    int pos = 0;

    terminal_clear();
    terminal_writestring_nl("OpenSYS OpenShell v1.0");
    terminal_writestring_nl("OpenFS filesystem ready.");
    terminal_writestring_nl("Type 'help' for commands.\n");

    while (1) {
        terminal_writestring("> ");

        pos = 0;
        while (1) {
            if (ps2_keyboard_has_key()) {
                char c = ps2_keyboard_getc();

                if (c == '\n') {
                    terminal_putchar('\n');
                    cmd_buffer[pos] = 0;
                    break;
                } else if (c == '\b' && pos > 0) {
                    pos--;
                    terminal_putchar('\b');
                    terminal_putchar(' ');
                    terminal_putchar('\b');
                } else if (c >= ' ' && pos < MAX_CMD_LEN - 1) {
                    cmd_buffer[pos++] = c;
                    terminal_putchar(c);
                }
            }

            for (volatile int i = 0; i < 1000; i++);
        }

        process_command(cmd_buffer);
    }
}
