#include "spoof_fgquery.h"

#include <imgui.h>
#include <console.h>
#include <hook.h>
#include <pattern_scanner.h>
#include <cstddef>
#include <cstdint>
#include <Windows.h>
#include <string>

enum class ptrn_ver
{
    UNKNOWN,
    V3,
    V4,
};

static bool window_visible = false;

static bool      spoof_title_active = false;
static bool      spoof_title_available = false;
static ptrn_ver  spoof_title_mode = ptrn_ver::UNKNOWN;
static char      spoof_title_buffer[MAX_PATH] = { L'\0' };
static void     *spoof_title_call_instruction = nullptr;

static int(__stdcall *o_GetWindowTextGeneric)(HWND, void *, int) = nullptr;
static int __stdcall hk_GetWindowTextGeneric(HWND hwnd, void *string, int maxcount)
{
    if (!spoof_title_active)
        return o_GetWindowTextGeneric(hwnd, string, maxcount);

    if (spoof_title_mode == ptrn_ver::V3)
    {
        strcpy_s(reinterpret_cast<char *>(string), maxcount, spoof_title_buffer);
    }
    else if (spoof_title_mode == ptrn_ver::V4)
    {
        wchar_t w_buff[MAX_PATH] = { L'\0' };
        MultiByteToWideChar(CP_UTF8, NULL, spoof_title_buffer, -1, w_buff, MAX_PATH);
        wcscpy_s(reinterpret_cast<wchar_t *>(string), maxcount, w_buff);
    }

    return static_cast<int>(strlen(spoof_title_buffer));
}

static bool spoof_class_active = false;
static bool spoof_class_available = false;
static char spoof_class_buffer[MAX_PATH] = { L'\0' };

static bool spoof_procname_active = false;
static bool spoof_procname_available = false;
static char spoof_procname_buffer[MAX_PATH] = { L'\0' };

bool spoof_fgquery_load(ldr_data_table_entry *hndy_entry)
{
    console::status_print stat_load_title("Loading Application Title spoofer...");

    std::cout << "\n\tScanning for GetWindowText pattern... ";
    if (std::uint8_t *title_ptrn_result_v3a = pattern_scan(hndy_entry->dll_base, hndy_entry->size_of_image, "\x68\x00\x00\x00\x00\x8B\x45\xF8\xE8\x00\x00\x00\x00\x50\x8B\x85", "x????xxxx????xxx"); title_ptrn_result_v3a)
    {
        spoof_title_mode = ptrn_ver::V3;
        spoof_title_call_instruction = title_ptrn_result_v3a + 0x15;
    }
    else if (std::uint8_t *title_ptrn_result_v4w = pattern_scan(hndy_entry->dll_base, hndy_entry->size_of_image, "\x68\x00\x00\x00\x00\x8B\x45\xFC\xE8\x00\x00\x00\x00\x50\x8B\x85", "x????xxxx????xxx"); title_ptrn_result_v4w)
    {
        spoof_title_mode = ptrn_ver::V4;
        spoof_title_call_instruction = title_ptrn_result_v4w + 0x15;
    }
    
    if (spoof_title_call_instruction)
    {
        std::cout << "\n\tHooking call instruction... "
                     "\n\t\tCall instruction: 0x" << spoof_title_call_instruction
                  << "\n\t\t    Hook address: 0x" << hk_GetWindowTextGeneric;

        if (hook_nearcall86(spoof_title_call_instruction, hk_GetWindowTextGeneric, reinterpret_cast<void **>(&o_GetWindowTextGeneric)))
        {
            std::cout << "\n\t\tOriginal address: 0x" << o_GetWindowTextGeneric;
            spoof_title_available = true;
        }
    }

    if (o_GetWindowTextGeneric)
        stat_load_title.ok();
    else
        stat_load_title.fail();

    return true;
}

bool spoof_fgquery_unload()
{
    std::cout << "Unloading handycafe foreground query...";

    spoof_title_active = false;
    window_visible = false;
    spoof_title_available = false;
    spoof_title_mode = ptrn_ver::UNKNOWN;
    spoof_title_call_instruction = nullptr;
    ZeroMemory(spoof_title_buffer, sizeof(spoof_title_buffer));

    if (o_GetWindowTextGeneric)
    {
        console::status_print("Unhooking call instruction for GetWindowText...").autoset(hook_nearcall86(spoof_title_call_instruction, o_GetWindowTextGeneric, nullptr));
        o_GetWindowTextGeneric = nullptr;
    }

    spoof_class_active = false;
    spoof_class_available = false;
    ZeroMemory(spoof_class_buffer, sizeof(spoof_class_buffer));

    spoof_procname_active = false;
    spoof_procname_available = false;
    ZeroMemory(spoof_procname_buffer, sizeof(spoof_procname_buffer));

    return true;
}

bool spoof_fgquery_is_loaded()
{
    return spoof_title_available;
}

void spoof_fgquery_toggle_window()
{
    window_visible = !window_visible;
}

void spoof_fgquery_draw_window()
{
    if (!window_visible)
        return;
    
    if (ImGui::Begin("handycafe: Spoof Foreground Query", &window_visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
    {
        ImGui::InputText("##spoof_title_buffer", spoof_title_buffer, sizeof(spoof_title_buffer));
        ImGui::SameLine();
        ImGui::Checkbox("Spoof Title", &spoof_title_active);

        ImGui::InputText("##spoof_class_buffer", spoof_class_buffer, sizeof(spoof_class_buffer));
        ImGui::SameLine();
        ImGui::Checkbox("Spoof Class name", &spoof_class_active);

        ImGui::InputText("##spoof_procname_buffer", spoof_procname_buffer, sizeof(spoof_procname_buffer));
        ImGui::SameLine();
        ImGui::Checkbox("Spoof Process name", &spoof_procname_active);
    }
    ImGui::End();
}
