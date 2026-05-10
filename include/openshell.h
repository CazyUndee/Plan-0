/*
 * openshell.h - OpenSYS Natural Language Shell Interface
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

#ifndef OPENSHELL_H
#define OPENSHELL_H

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
void openshell_init(void);
void openshell_run(void);
void openshell_execute_command(const char* command);
void openshell_register_command(const char* name, shell_cmd_type_t type);

// Command execution
shell_result_t openshell_list_files(void);
shell_result_t openshell_create_file(const char* name);
shell_result_t openshell_delete_file(const char* name);
shell_result_t openshell_copy_file(const char* src, const char* dst);
shell_result_t openshell_move_file(const char* src, const char* dst);
shell_result_t openshell_write_file(const char* name, const char* content);
shell_result_t openshell_read_file(const char* name);
shell_result_t openshell_get_file_info(const char* name);

// System commands
shell_result_t openshell_show_memory(void);
shell_result_t openshell_show_processes(void);
shell_result_t openshell_show_system_info(void);

// Window management
shell_result_t openshell_open_window(const char* title, int x, int y, int width, int height);
shell_result_t openshell_close_window(const char* window_id);
shell_result_t openshell_move_window(const char* window_id, int x, int y);
shell_result_t openshell_focus_window(const char* window_id);
shell_result_t openshell_list_windows(void);

// Utility functions
void openshell_clear_screen(void);
void openshell_show_help(void);
void openshell_echo(const char* text);
void openshell_show_version(void);

#endif // OPENSHELL_H
