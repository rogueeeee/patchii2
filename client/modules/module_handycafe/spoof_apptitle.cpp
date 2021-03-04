#include "spoof_apptitle.h"

#include <imgui.h>
#include <console.h>
#include <hook.h>
#include <pattern_scanner.h>
#include <cstddef>
#include <cstdint>
#include <Windows.h>
#include <string>

enum class spoof_mode
{
    UNAVAILABLE,
    VER3,
    VER4
};

static spoof_mode mode = spoof_mode::UNAVAILABLE;
static bool window_visible = false;
static bool active = false;
static char spoof_txt_buff[MAX_PATH] = { "File Explorer" };

void *call_address = nullptr;
int(__stdcall*o_GetWindowTextGeneric)(HWND, void*, int) = nullptr;
int __stdcall hk_GetWindowTextGeneric(HWND hwnd, void *string, int maxcount)
{
    if (!active)
        return o_GetWindowTextGeneric(hwnd, string, maxcount);

    if (mode == spoof_mode::VER3)
    {
        strcpy_s(reinterpret_cast<char *>(string), maxcount, spoof_txt_buff);
    }
    else if (mode == spoof_mode::VER4)
    {
        wchar_t w_buff[MAX_PATH] = { L'\0' };
        MultiByteToWideChar(CP_UTF8, NULL, spoof_txt_buff, -1, w_buff, MAX_PATH);
        wcscpy_s(reinterpret_cast<wchar_t *>(string), maxcount, w_buff);
    }

    return strlen(spoof_txt_buff);
}

bool load_mode_by_match(ldr_data_table_entry *hndy_entry, void *&result_dest)
{
    console::status_print stat_sigv3a("Looking for pattern (version 3)...");
    std::uint8_t *ptrn_result_v3a = pattern_scan(hndy_entry->dll_base, hndy_entry->size_of_image, "\x68\x00\x00\x00\x00\x8B\x45\xF8\xE8\x00\x00\x00\x00\x50\x8B\x85", "x????xxxx????xxx");
    if (stat_sigv3a.autoset(ptrn_result_v3a))
    {
        result_dest = ptrn_result_v3a + 0x15;
        mode = spoof_mode::VER3;
        stat_sigv3a.ok();
        return true;
    }

    console::status_print stat_sigv4w("Looking for pattern (version 4)...");
    std::uint8_t *ptrn_result_v4w = pattern_scan(hndy_entry->dll_base, hndy_entry->size_of_image, "\x68\x00\x00\x00\x00\x8B\x45\xFC\xE8\x00\x00\x00\x00\x50\x8B\x85", "x????xxxx????xxx");
    if (stat_sigv4w.autoset(ptrn_result_v4w))
    {
        result_dest = ptrn_result_v4w + 0x15;
        mode = spoof_mode::VER4;
        stat_sigv4w.ok();
        return true;
    }

    return false;
}

bool spoof_apptitle_load(ldr_data_table_entry *hndy_entry)
{
    console::status_print stat_load("Loading Spoof Application Title");
    
    if (!load_mode_by_match(hndy_entry, call_address))
    {
        stat_load.fail();
        return false;
    }

    console::status_print stat_hk_gwt("Hooking GetWindowText call");
    std::cout << "\n\tCall instruction: 0x" << call_address;
    std::cout << "\n\t    Hook address: 0x" << hk_GetWindowTextGeneric;
    if (!stat_hk_gwt.autoset(hook_nearcall86(call_address, hk_GetWindowTextGeneric, reinterpret_cast<void **>(&o_GetWindowTextGeneric))))
        return false;

    std::cout << "\n\tOriginal address: 0x" << o_GetWindowTextGeneric;

    stat_load.ok();
    return true;
}

bool spoof_apptitle_unload()
{
    if (mode == spoof_mode::UNAVAILABLE)
        return true;
    
    std::cout << "\nUnloading Spoof Application Title";
    
    console::status_print("Unhooking GetWindowText call").autoset(hook_nearcall86(call_address, o_GetWindowTextGeneric, nullptr));

    call_address   = nullptr;
    mode           = spoof_mode::UNAVAILABLE;
    window_visible = false;
    active         = false;

    return true;
}

bool spoof_apptitle_is_loaded()
{
    return mode != spoof_mode::UNAVAILABLE;
}

void spoof_apptitle_toggle_window()
{
    window_visible = !window_visible;
}

void spoof_apptitle_draw_window()
{
    if (!window_visible)
        return;

    if (ImGui::Begin("handycafe: Spoof Application Title", &window_visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
    {
        ImGui::Checkbox("Spoof:", &active);
        ImGui::SameLine();
        ImGui::InputText("##spoof_txt_buff", spoof_txt_buff, sizeof(spoof_txt_buff));
    }
    ImGui::End();
}
