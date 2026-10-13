/*
 * PROJECT:   LVGL PC Simulator using Visual Studio
 * FILE:      LVGL.Simulator.cpp
 * PURPOSE:   Implementation for LVGL ported to Windows Desktop
 *            程序主入口
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

#include <Windows.h>
#include <profileapi.h>

#include "resource.h"

#if _MSC_VER >= 1200
 // Disable compilation warnings.
#pragma warning(push)
// nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)
// 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4244)
#endif

#include <lvgl/lvgl.h>
#include <lvgl/examples/lv_examples.h>
#include <lvgl/demos/lv_demos.h>
#include "lv_drivers/win32drv/win32drv.h"

#if _MSC_VER >= 1200
// Restore compilation warnings.
#pragma warning(pop)
#endif

#include <stdio.h>

extern "C" void app_main_re(void);

//----------------------------------------------------------------------
static int64_t g_timeProcessSt;

//----------------------------------------------------------------------

bool single_display_mode_initialization()
{
    if (!lv_win32_init(
        GetModuleHandleW(NULL),
        SW_SHOW,
        SCREEN_WIDTH, 
        SCREEN_HEIGHT,
        LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDI_LVGL))))
    {
        return false;
    }

    lv_win32_add_all_input_devices_to_group(NULL);

    return true;
}

#include <process.h>

HANDLE g_window_mutex = NULL;
bool g_initialization_status = false;

#define LVGL_SIMULATOR_MAXIMUM_DISPLAYS 16
HWND g_display_window_handles[LVGL_SIMULATOR_MAXIMUM_DISPLAYS];

unsigned int __stdcall lv_win32_window_thread_entrypoint(
    void* raw_parameter)
{
	LV_LOG_USER("space func!");
    return 0;
}

bool multiple_display_mode_initialization()
{
    if (!lv_win32_init_window_class())
    {
        return false;
    }

    for (size_t i = 0; i < LVGL_SIMULATOR_MAXIMUM_DISPLAYS; ++i)
    {
        g_initialization_status = false;

        g_window_mutex = CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);

        _beginthreadex(
            NULL,
            0,
            lv_win32_window_thread_entrypoint,
            &i,
            0,
            NULL);

        WaitForSingleObjectEx(g_window_mutex, INFINITE, FALSE);

        CloseHandle(g_window_mutex);

        if (!g_initialization_status)
        {
            return false;
        }
    }

    lv_win32_window_context_t* context = (lv_win32_window_context_t*)(
        lv_win32_get_window_context(g_display_window_handles[0]));
    if (context)
    {
        lv_win32_pointer_device_object = context->mouse_device_object;
        lv_win32_keypad_device_object = context->keyboard_device_object;
        lv_win32_encoder_device_object = context->mousewheel_device_object;
    }

    lv_win32_add_all_input_devices_to_group(NULL);

    return true;
}

//----------------------- 

extern "C" long long _sys_time_ms_now()
{
    static LARGE_INTEGER s_frequency;
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else {
        return GetTickCount();
    }
}

extern "C" int32_t zb_get_time_ms_since_boot(void)
{
    int64_t timeNow = _sys_time_ms_now();
    return (int32_t)(timeNow - g_timeProcessSt);
}

extern "C" int64_t zb_get_time_us_since_boot(void)
{
    int64_t timeNow = _sys_time_ms_now();
    return (timeNow - g_timeProcessSt) * 1000LL;
}

//----------------------------------------------------------------------


int main()
{
#ifdef _WIN64
    #error "you should select x86 mode!"
#endif
    SetConsoleOutputCP(65001);
    g_timeProcessSt = _sys_time_ms_now();
    lv_init();

    if (!single_display_mode_initialization())
    {
        return -1;
    }

    // ----------------------------------
    // Task handler loop
    // ----------------------------------
    app_main_re();
    return 0;
}

