/*
 * shell.h - Natural Language Shell Interface
 *
 * Copyright (C) 2026 CazyUndee
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

// Shell command types
typedef enum {
    SHELL_CMD_LIST,
    SHELL_CMD_CREATE,
    SHELL_CMD_DELETE,
    SHELL_CMD_COPY,
    SHELL_CMD_MOVE,
    SHELL_CMD_WRITE,
    SHELL_CMD_READ,
    SHELL_CMD_INFO,
    SHELL_CMD_SYSTEM,
    SHELL_CMD_WINDOW,
    SHELL_CMD_PROCESS
} shell_cmd_type_t;

// Shell result
typedef struct {
    int success;
    char error[256];
    char output[512];
} shell_result_t;

// Core API
void shell_init(void);
void shell_run(void);
void shell_execute_command(const char* command);
void shell_register_command(const char* name, shell_cmd_type_t type);

// Command execution
shell_result_t shell_list_files(void);
shell_result_t shell_create_file(const char* name);
shell_result_t shell_delete_file(const char* name);
shell_result_t shell_copy_file(const char* src, const char* dst);
shell_result_t shell_move_file(const char* src, const char* dst);
shell_result_t shell_write_file(const char* name, const char* content);
shell_result_t shell_read_file(const char* name);
shell_result_t shell_get_file_info(const char* name);

// System commands
shell_result_t shell_show_memory(void);
shell_result_t shell_show_processes(void);
shell_result_t shell_show_system_info(void);

// Window management
shell_result_t shell_open_window(const char* title, int x, int y, int width, int height);
shell_result_t shell_close_window(const char* window_id);
shell_result_t shell_move_window(const char* window_id, int x, int y);
shell_result_t shell_focus_window(const char* window_id);
shell_result_t shell_list_windows(void);

// Utility functions
void shell_clear_screen(void);
void shell_show_help(void);
void shell_echo(const char* text);
void shell_show_version(void);

#endif // SHELL_H
