/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2022 Rachel Powers <508861+Ryex@users.noreply.github.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "WindowsConsole.h"
#include <system_error>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <consoleapi.h>
#include <fcntl.h>
#include <fileapi.h>
#include <io.h>
#include <stdio.h>
#include <cstddef>
#include <iostream>

namespace console {

void RedirectHandle(DWORD handle, FILE* stream, const char* mode)
{
    HANDLE stdHandle = GetStdHandle(handle);
    if (stdHandle != INVALID_HANDLE_VALUE) {
        int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
        if (fileDescriptor != -1) {
            FILE* file = _fdopen(fileDescriptor, mode);
            if (file != NULL) {
                int dup2Result = _dup2(_fileno(file), _fileno(stream));
                if (dup2Result == 0) {
                    setvbuf(stream, NULL, _IONBF, 0);
                }
            }
        }
    }
}

void BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr)
{

    if (bindStdIn) {
        FILE* dummyFile;
        freopen_s(&dummyFile, "nul", "r", stdin);
    }
    if (bindStdOut) {
        FILE* dummyFile;
        freopen_s(&dummyFile, "nul", "w", stdout);
    }
    if (bindStdErr) {
        FILE* dummyFile;
        freopen_s(&dummyFile, "nul", "w", stderr);
    }

    if (bindStdIn) {
        RedirectHandle(STD_INPUT_HANDLE, stdin, "r");
    }

    if (bindStdOut) {
        RedirectHandle(STD_OUTPUT_HANDLE, stdout, "w");
    }

    if (bindStdErr) {
        RedirectHandle(STD_ERROR_HANDLE, stderr, "w");
    }

    if (bindStdIn) {
        std::wcin.clear();
        std::cin.clear();
    }
    if (bindStdOut) {
        std::wcout.clear();
        std::cout.clear();
    }
    if (bindStdErr) {
        std::wcerr.clear();
        std::cerr.clear();
    }
}

bool AttachWindowsConsole()
{
    auto stdinType = GetFileType(GetStdHandle(STD_INPUT_HANDLE));
    auto stdoutType = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
    auto stderrType = GetFileType(GetStdHandle(STD_ERROR_HANDLE));

    bool bindStdIn = false;
    bool bindStdOut = false;
    bool bindStdErr = false;

    if (stdinType == FILE_TYPE_CHAR || stdinType == FILE_TYPE_UNKNOWN) {
        bindStdIn = true;
    }
    if (stdoutType == FILE_TYPE_CHAR || stdoutType == FILE_TYPE_UNKNOWN) {
        bindStdOut = true;
    }
    if (stderrType == FILE_TYPE_CHAR || stderrType == FILE_TYPE_UNKNOWN) {
        bindStdErr = true;
    }

    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        BindCrtHandlesToStdHandles(bindStdIn, bindStdOut, bindStdErr);
        return true;
    }

    return false;
}

std::error_code EnableAnsiSupport()
{

    HANDLE console_handle = CreateFileW(L"CONOUT$", FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
    if (console_handle == INVALID_HANDLE_VALUE) {
        return std::error_code(GetLastError(), std::system_category());
    }

    DWORD console_mode;
    if (0 == GetConsoleMode(console_handle, &console_mode)) {
        return std::error_code(GetLastError(), std::system_category());
    }

    if ((console_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {

        if (0 == SetConsoleMode(console_handle, console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            return std::error_code(GetLastError(), std::system_category());
        }
    }

    return {};
}

void FreeWindowsConsole()
{
    fclose(stdout);
    fclose(stdin);
    fclose(stderr);
    FreeConsole();
}

WindowsConsoleGuard::WindowsConsoleGuard() : m_consoleAttached(false)
{
    if (console::AttachWindowsConsole()) {
        m_consoleAttached = true;
        if (auto err = console::EnableAnsiSupport(); err) {
            std::cout << "Error setting up ansi console" << err.message() << std::endl;
        }
    }
}

WindowsConsoleGuard::~WindowsConsoleGuard()
{

    if (m_consoleAttached) {
        console::FreeWindowsConsole();
    }
}

}
