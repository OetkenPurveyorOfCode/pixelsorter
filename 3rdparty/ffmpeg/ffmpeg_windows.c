// stolen and modified from tsodings musializer (license below)
/*
Copyright 2023 Alexey Kutepov <reximkut@gmail.com> and Musializer Contributors

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>

#include "ffmpeg.h"

struct FFMPEG {
    HANDLE hProcess;
    HANDLE hPipeWrite;
};

FFMPEG *ffmpeg_start_rendering(size_t width, size_t height, size_t fps, char* videoname)
{
    HANDLE pipe_read;
    HANDLE pipe_write;

    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    if (!CreatePipe(&pipe_read, &pipe_write, &saAttr, 0)) {
        printf("ERROR: Could not create pipe to ffmpeg. System Error Code: %d", (int)GetLastError());
        return NULL;
    }

    if (!SetHandleInformation(pipe_write, HANDLE_FLAG_INHERIT, 0)) {
        printf("ERROR: Could not mark write pipe as non-inheritable. System Error Code: %d", (int)GetLastError());
        return NULL;
    }
    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if (siStartInfo.hStdError == INVALID_HANDLE_VALUE) {
        printf("ERROR: Could get standard error handle for the child. System Error Code: %d", (int)GetLastError());
        return NULL;
    }
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (siStartInfo.hStdOutput == INVALID_HANDLE_VALUE) {
        printf("ERROR: Could get standard output handle for the child. System Error Code: %d", (int)GetLastError());
        return NULL;
    }
    siStartInfo.hStdInput = pipe_read;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // TODO: use String_Builder in here
    // TODO: sanitize user input through sound_file_path
    char cmd_buffer[1024*2];
    snprintf(cmd_buffer, sizeof(cmd_buffer), 
        "ffmpeg.exe "
        "-loglevel verbose "
        "-y " // overwrite output files
        "-f rawvideo -pix_fmt bgra -s %dx%d -r %d -i - -c:v libx264 -vb 7000k -pix_fmt yuv420p %s"
    , (int)width, (int)height, (int)fps, videoname);

    if (!CreateProcess(NULL, cmd_buffer, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
        printf("ERROR: Could not create child process. System Error Code: %d", (int)GetLastError());

        CloseHandle(pipe_write);
        CloseHandle(pipe_read);

        return NULL;
    }

    CloseHandle(pipe_read);
    CloseHandle(piProcInfo.hThread);

    FFMPEG *ffmpeg = malloc(sizeof(FFMPEG));
    assert(ffmpeg != NULL && "OOM");
    ffmpeg->hProcess = piProcInfo.hProcess;
    ffmpeg->hPipeWrite = pipe_write;
    return ffmpeg;
}

bool ffmpeg_send_frame_flipped(FFMPEG *ffmpeg, void *data, size_t width, size_t height)
{
    DWORD written;

    for (size_t y = height; y > 0; --y) {
        // TODO: handle ERROR_IO_PENDING
        if (!WriteFile(ffmpeg->hPipeWrite, (uint32_t*)data + (y - 1)*width, sizeof(uint32_t)*width, &written, NULL)) {
            printf("ERROR: failed to write into ffmpeg pipe. System Error Code: %d", (int)GetLastError());
            return false;
        }
    }
    return true;
}

bool ffmpeg_send_frame(FFMPEG *ffmpeg, void *data, size_t width, size_t height)
{
    DWORD written;
    if (!WriteFile(ffmpeg->hPipeWrite, (uint32_t*)data, sizeof(uint32_t)*width*height, &written, NULL)) {
        printf("ERROR: failed to write into ffmpeg pipe. System Error Code: %d", (int)GetLastError());
        return false;
    }
    return true;
}

bool ffmpeg_end_rendering(FFMPEG *ffmpeg, bool cancel)
{
    HANDLE hPipeWrite = ffmpeg->hPipeWrite;
    HANDLE hProcess = ffmpeg->hProcess;
    free(ffmpeg);

    FlushFileBuffers(hPipeWrite);
    CloseHandle(hPipeWrite);

    if (cancel) TerminateProcess(hProcess, 69);

    if (WaitForSingleObject(hProcess, INFINITE) == WAIT_FAILED) {
        printf("ERROR: could not wait on child process. System Error Code: %d", (int)GetLastError());
        CloseHandle(hProcess);
        return false;
    }

    DWORD exit_status;
    if (GetExitCodeProcess(hProcess, &exit_status) == 0) {
        printf("ERROR: could not get process exit code. System Error Code: %d", (int)GetLastError());
        CloseHandle(hProcess);
        return false;
    }

    if (exit_status != 0) {
        printf("ERROR: command exited with exit code %lu", (unsigned long)exit_status);
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);

    return true;
}

// TODO: where can we find this symbol for the Windows build?
void __imp__wassert() {}
