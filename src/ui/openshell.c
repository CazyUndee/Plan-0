/*
 * shell.c - Natural Language Shell
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
#include "../include/ui_command.h"

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

static void cmd_copy(const char* src, const char* dst) {
    if (!src || !*src || !dst || !*dst) {
        terminal_writestring_nl("  Usage: copy <source> <destination>");
        return;
    }
    
    // Build source path
    char src_path[256];
    if (src[0] == '/') {
        int len = 0;
        while (src[len] && len < 255) {
            src_path[len] = src[len];
            len++;
        }
        src_path[len] = 0;
    } else {
        src_path[0] = '/';
        int len = 1;
        while (src[len-1] && len < 255) {
            src_path[len] = src[len-1];
            len++;
        }
        src_path[len] = 0;
    }
    
    // Build destination path
    char dst_path[256];
    if (dst[0] == '/') {
        int len = 0;
        while (dst[len] && len < 255) {
            dst_path[len] = dst[len];
            len++;
        }
        dst_path[len] = 0;
    } else {
        dst_path[0] = '/';
        int len = 1;
        while (dst[len-1] && len < 255) {
            dst_path[len] = dst[len-1];
            len++;
        }
        dst_path[len] = 0;
    }
    
    // Open source file
    fs_file_t* src_file = fs_open(src_path, 0);
    if (!src_file) {
        terminal_writestring_nl("  Error: Source file not found");
        return;
    }
    
    // Create destination file
    fs_file_t* dst_file = fs_open(dst_path, 1);
    if (!dst_file) {
        fs_close(src_file);
        terminal_writestring_nl("  Error: Could not create destination file");
        return;
    }
    
    // Copy data
    char buf[256];
    size_t total_copied = 0;
    while (total_copied < src_file->size) {
        size_t to_read = src_file->size - total_copied > 256 ? 256 : src_file->size - total_copied;
        size_t read = fs_read(src_file, buf, to_read);
        if (read == 0) break;
        
        size_t written = fs_write(dst_file, buf, read);
        if (written != read) {
            terminal_writestring_nl("  Error: Write failed during copy");
            break;
        }
        
        total_copied += written;
    }
    
    fs_close(src_file);
    fs_close(dst_file);
    
    terminal_writestring("  Copied: ");
    terminal_put_dec(total_copied);
    terminal_writestring_nl(" bytes");
}

static void cmd_move(const char* src, const char* dst) {
    if (!src || !*src || !dst || !*dst) {
        terminal_writestring_nl("  Usage: move <source> <destination>");
        return;
    }
    
    // Copy the file
    cmd_copy(src, dst);
    
    // Delete the original
    char src_path[256];
    if (src[0] == '/') {
        int len = 0;
        while (src[len] && len < 255) {
            src_path[len] = src[len];
            len++;
        }
        src_path[len] = 0;
    } else {
        src_path[0] = '/';
        int len = 1;
        while (src[len-1] && len < 255) {
            src_path[len] = src[len-1];
            len++;
        }
        src_path[len] = 0;
    }
    
    if (fs_unlink(src_path) >= 0) {
        terminal_writestring_nl("  Move completed");
    } else {
        terminal_writestring_nl("  Warning: Original file not deleted");
    }
}

static void cmd_append(const char* name, const char* content) {
    if (!name || !*name) {
        terminal_writestring_nl("  Usage: append <filename> <content>");
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
        terminal_writestring_nl("  Error: Could not open file");
        return;
    }
    
    if (content && *content) {
        size_t len = k_strlen(content);
        size_t written = fs_write(file, content, len);
        fs_close(file);
        
        terminal_writestring("  Appended ");
        terminal_put_dec(written);
        terminal_writestring_nl(" bytes");
    } else {
        fs_close(file);
        terminal_writestring_nl("  No content to append");
    }
}

static void cmd_rename(const char* old_name, const char* new_name) {
    if (!old_name || !*old_name || !new_name || !*new_name) {
        terminal_writestring_nl("  Usage: rename <oldname> <newname>");
        return;
    }
    
    // Use move command for rename
    cmd_move(old_name, new_name);
}

static void cmd_find(const char* pattern) {
    if (!pattern || !*pattern) {
        terminal_writestring_nl("  Usage: find <pattern>");
        return;
    }
    
    terminal_writestring_nl("");
    terminal_writestring("  Searching for: \"");
    terminal_writestring(pattern);
    terminal_writestring_nl("\"");
    terminal_writestring_nl("");
    
    // Simple directory search - would need filesystem support for recursive search
    int count = 0;
    // This would need to be implemented with actual filesystem search
    terminal_writestring("  Found ");
    terminal_put_dec(count);
    terminal_writestring_nl(" matching files");
    terminal_writestring_nl("");
}

static void cmd_echo(const char* text) {
    if (text && *text) {
        terminal_writestring("  ");
        terminal_writestring_nl(text);
    } else {
        terminal_putchar('\n');
    }
}

static void cmd_open_window(const char* app_id) {
    command_result_t result = cmd_open_window(app_id);
    if (!result.success) {
        terminal_writestring("  Error: ");
        terminal_writestring_nl(result.error);
    }
}

static void cmd_close_window(const char* window_id) {
    command_result_t result = cmd_close_window(window_id);
    if (!result.success) {
        terminal_writestring("  Error: ");
        terminal_writestring_nl(result.error);
    }
}

static void cmd_move_window(const char* window_id, int x, int y) {
    command_result_t result = cmd_move_window(window_id, x, y);
    if (!result.success) {
        terminal_writestring("  Error: ");
        terminal_writestring_nl(result.error);
    }
}

static void cmd_focus_window(const char* window_id) {
    command_result_t result = cmd_focus_window(window_id);
    if (!result.success) {
        terminal_writestring("  Error: ");
        terminal_writestring_nl(result.error);
    }
}

static void cmd_list_windows(void) {
    command_result_t result = cmd_list_windows();
    if (!result.success) {
        terminal_writestring("  Error: ");
        terminal_writestring_nl(result.error);
    }
}

static void cmd_list_processes(void) {
    command_result_t result = cmd_list_processes();
    if (!result.success) {
        terminal_writestring("  Error: ");
        terminal_writestring_nl(result.error);
    }
}

static void cmd_show_system_info(void) {
    command_result_t result = cmd_show_system_info();
    if (!result.success) {
        terminal_writestring("  Error: ");
        terminal_writestring_nl(result.error);
    }
}

static void cmd_version(void) {
    terminal_writestring_nl("");
    terminal_writestring_nl("  OpenSYS OS v0.4.0");
    terminal_writestring_nl("  OpenKernel - 64-bit Operating System");
    terminal_writestring_nl("  OpenFS - NTFS-style filesystem");
    terminal_writestring_nl("  OpenShell - Natural language interface");
    terminal_writestring_nl("  Copyright (C) 2026 CazyUndee");
    terminal_writestring_nl("  Licensed under GPL-3.0");
    terminal_writestring_nl("");
}

static void cmd_system_info(void) {
    terminal_writestring_nl("");
    terminal_writestring_nl("  System Information:");
    terminal_writestring_nl("  ------------------");
    
    // Memory info
    uint64_t total = pmm_get_total() / (1024 * 1024);
    uint64_t free = pmm_get_free() / (1024 * 1024);
    terminal_writestring("  Total RAM: ");
    terminal_put_dec(total);
    terminal_writestring(" MB, Free: ");
    terminal_put_dec(free);
    terminal_writestring_nl(" MB");
    
    // Process info
    extern int process_get_count(void);
    int proc_count = process_get_count();
    terminal_writestring("  Running processes: ");
    terminal_put_dec(proc_count);
    terminal_writestring_nl("");
    
    // Date/Time
    rtc_time_t t;
    rtc_read_time(&t);
    terminal_writestring("  Date: ");
    terminal_put_dec(t.month);
    terminal_putchar('/');
    terminal_put_dec(t.day);
    terminal_putchar('/');
    terminal_put_dec(t.century);
    terminal_put_dec(t.year);
    terminal_writestring("  Time: ");
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
    terminal_writestring_nl("  OpenShell Natural Language Commands:");
    terminal_writestring_nl("  ---------------------------------");
    terminal_writestring_nl("  File Operations:");
    terminal_writestring_nl("    list              - show all files");
    terminal_writestring_nl("    create <name>     - create new file");
    terminal_writestring_nl("    make directory <name> - create directory");
    terminal_writestring_nl("    delete <name>     - delete file or directory");
    terminal_writestring_nl("    copy <source> <destination> - copy file");
    terminal_writestring_nl("    move <source> <destination> - move file");
    terminal_writestring_nl("    rename <old> <new> - rename file");
    terminal_writestring_nl("    write <name> <text> - write text to file");
    terminal_writestring_nl("    append <name> <text> - add text to file");
    terminal_writestring_nl("    read <name>       - display file contents");
    terminal_writestring_nl("    information about <name> - show file details");
    terminal_writestring_nl("    find <pattern>    - search for files");
    terminal_writestring_nl("");
    terminal_writestring_nl("  Window Management:");
    terminal_writestring_nl("    open window <app> - open application window");
    terminal_writestring_nl("    close window <id> - close window");
    terminal_writestring_nl("    move window <id> <x> <y> - move window");
    terminal_writestring_nl("    focus window <id> - focus window");
    terminal_writestring_nl("    list windows     - show all open windows");
    terminal_writestring_nl("");
    terminal_writestring_nl("  System Commands:");
    terminal_writestring_nl("    show processes    - list running processes");
    terminal_writestring_nl("    show memory       - display memory usage");
    terminal_writestring_nl("    system information - detailed system status");
    terminal_writestring_nl("    current date time - show date and time");
    terminal_writestring_nl("    version          - show OS version");
    terminal_writestring_nl("");
    terminal_writestring_nl("  Shell Utilities:");
    terminal_writestring_nl("    echo <text>       - display text");
    terminal_writestring_nl("    clear screen      - clear terminal");
    terminal_writestring_nl("    help              - show this help");
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

    if (cmd_equals(cmd, "list")) {
        cmd_list();
    }
    else if (cmd_equals(cmd, "open window") || cmd_equals(cmd, "open application")) {
        char* app_id = arg1;
        while (*app_id && *app_id != ' ') app_id++;
        if (*app_id == ' ') {
            *app_id = 0;
            app_id++;
            while (*app_id == ' ') app_id++;
        }
        cmd_open_window(app_id);
    }
    else if (cmd_equals(cmd, "close window")) {
        cmd_close_window(arg1);
    }
    else if (cmd_equals(cmd, "move window")) {
        char* win_id = arg1;
        char* x_str = arg2;
        while (*x_str && *x_str != ' ') x_str++;
        if (*x_str == ' ') {
            *x_str = 0;
            x_str++;
            while (*x_str == ' ') x_str++;
        }
        char* y_str = x_str;
        while (*y_str && *y_str != ' ') y_str++;
        if (*y_str == ' ') {
            *y_str = 0;
            y_str++;
            while (*y_str == ' ') y_str++;
        }
        
        if (*win_id && *x_str && *y_str) {
            int x = 0, y = 0;
            char* p = x_str;
            while (*p >= '0' && *p <= '9') {
                x = x * 10 + (*p - '0');
                p++;
            }
            p = y_str;
            while (*p >= '0' && *p <= '9') {
                y = y * 10 + (*p - '0');
                p++;
            }
            cmd_move_window(win_id, x, y);
        } else {
            terminal_writestring_nl("  Usage: move window <id> <x> <y>");
        }
    }
    else if (cmd_equals(cmd, "focus window")) {
        cmd_focus_window(arg1);
    }
    else if (cmd_equals(cmd, "list windows") || cmd_equals(cmd, "show windows")) {
        cmd_list_windows();
    }
    else if (cmd_equals(cmd, "show processes")) {
        cmd_ps();
    }
    else if (cmd_equals(cmd, "current date time") || cmd_equals(cmd, "date time")) {
        cmd_date();
    }
    else if (cmd_equals(cmd, "create")) {
        cmd_create_file(arg1);
    }
    else if (cmd_equals(cmd, "make directory")) {
        cmd_mkdir(arg1);
    }
    else if (cmd_equals(cmd, "delete")) {
        cmd_delete(arg1);
    }
    else if (cmd_equals(cmd, "copy")) {
        cmd_copy(arg1, arg2);
    }
    else if (cmd_equals(cmd, "move")) {
        cmd_move(arg1, arg2);
    }
    else if (cmd_equals(cmd, "rename")) {
        cmd_rename(arg1, arg2);
    }
    else if (cmd_equals(cmd, "write")) {
        cmd_write_file(arg1, arg2);
    }
    else if (cmd_equals(cmd, "append")) {
        cmd_append(arg1, arg2);
    }
    else if (cmd_equals(cmd, "read")) {
        cmd_read_file(arg1);
    }
    else if (cmd_equals(cmd, "information about")) {
        cmd_file_info(arg1);
    }
    else if (cmd_equals(cmd, "find")) {
        cmd_find(arg1);
    }
    else if (cmd_equals(cmd, "echo")) {
        cmd_echo(arg1);
    }
    else if (cmd_equals(cmd, "show memory")) {
        cmd_show_memory();
    }
    else if (cmd_equals(cmd, "system information")) {
        cmd_system_info();
    }
    else if (cmd_equals(cmd, "version")) {
        cmd_version();
    }
    else if (cmd_equals(cmd, "clear screen")) {
        terminal_clear();
    }
    else if (cmd_equals(cmd, "help")) {
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

    // Initialize unified UI system
    ui_command_init();

    terminal_clear();
    terminal_writestring_nl("OpenSYS OpenShell v1.0");
    terminal_writestring_nl("OpenFS filesystem ready.");
    terminal_writestring_nl("Unified UI system initialized.");
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
