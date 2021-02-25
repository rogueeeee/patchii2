#include "api_int_terminateprocess.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <client/api_hooks.h>
#include <console.h>
#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>

struct mode_
{
    enum
    {
        DO_NOTHING,
        IMMEDIATELY_RETURN,
        FILTER_BY_NAME,
    };
};

static bool is_active      = false;
static bool window_visible = false;
static bool log_intercept   = false;
static int  mode           = mode_::DO_NOTHING;
static int  default_return = 1;
static std::vector<std::pair<std::string, bool>> proc_name_filter;

void cache_process()
{
    HANDLE proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (!proc_snap)
        return;
    
    std::unordered_map<std::string, bool> temp_proc_name_filter;
    PROCESSENTRY32W proc_entry = { sizeof(PROCESSENTRY32W) };

    while (Process32NextW(proc_snap, &proc_entry))
    {
        std::string mb_proc_name;
        {
            char _mb_proc_name[MAX_PATH] = { '\0' };
            WideCharToMultiByte(CP_UTF8, NULL, proc_entry.szExeFile, -1, _mb_proc_name, MAX_PATH, nullptr, nullptr);
            mb_proc_name = _mb_proc_name;
        }

        if (mb_proc_name.substr(mb_proc_name.length() - 4) != ".exe")
            continue;

        bool already_exists = false;
        bool existing_value = false;
        for (const auto &proc : proc_name_filter)
        {
            if (proc.first == mb_proc_name)
            {
                already_exists = true;
                existing_value = proc.second;
                break;
            }
        }

        temp_proc_name_filter[mb_proc_name] = already_exists ? existing_value : false;
    }

    proc_name_filter.clear();

    for (const auto& entry : temp_proc_name_filter)
        proc_name_filter.push_back(std::make_pair(entry.first, entry.second));

    CloseHandle(proc_snap);
}

void __stdcall apicb_TerminateProcess(api_hook_event &e, HANDLE &proc, UINT &exitcode)
{
    if (!is_active)
        return;
    
    char mod_name_buff[MAX_PATH] = { '\0' };
    if (log_intercept || mode == mode_::FILTER_BY_NAME)
    {
        if (!GetProcessImageFileNameA(proc, mod_name_buff, MAX_PATH))
            console::print_warning("Failed to obtain module file name. Error code: " + std::to_string(GetLastError()));
    }

    if (log_intercept)
    {
        std::cout << "\nAPI Intercept: TerminateProcess"
                  << "\n\tReturn Address: 0x"   << e.return_address
                  << "\n\t        Handle: 0x" << proc
                  << "\n\t     Exit code: "   << exitcode << " (Unsigned) / " << static_cast<int>(exitcode) << " (Signed)"
                  << "\n\t            ID: "   << GetProcessId(proc)
                  << "\n\t        Module: "   << mod_name_buff;
    }

    switch (mode)
    {
        case mode_::IMMEDIATELY_RETURN:
        {
            e.ret_val.i32 = default_return;
            e.flags |= api_hook_flags::END_CALLBACK | api_hook_flags::USE_EVENT_RETURN | api_hook_flags::DONT_CALL_ORIGINAL;
            break;
        }

        case mode_::FILTER_BY_NAME:
        {
            if (mod_name_buff[0] == '\0')
                break;
            
            const std::string mod_name_str(mod_name_buff);
            for (const auto &proc : proc_name_filter)
            {
                if (proc.second && proc.first.length() <= mod_name_str.length() && mod_name_str.find(proc.first) != std::string::npos)
                {
                    e.ret_val.i32 = 1;
                    e.flags |= api_hook_flags::END_CALLBACK | api_hook_flags::USE_EVENT_RETURN | api_hook_flags::DONT_CALL_ORIGINAL;
                    break;
                }
            }
        }
    }

}

bool api_int_terminateprocess_load()
{
    if (!console::status_print("[API Intercept] Adding API callback for TerminateProcess").autoset(patchii_apihooks_register("TerminateProcess", apicb_TerminateProcess)))
        return false;

    cache_process();
    return true;
}

bool api_int_terminateprocess_unload()
{
    if (console::status_print("[API Intercept] Removing API callback for TerminateProcess").autoset(patchii_apihooks_unregister("TerminateProcess", apicb_TerminateProcess)))
        return false;
    
    window_visible     = false;
    is_active          = false;
    log_intercept      = false;
    mode               = mode_::DO_NOTHING;
    
    proc_name_filter.clear();
    return true;
}

void api_int_terminateprocess_toggle_window()
{
    window_visible = !window_visible;
}

void api_int_terminateprocess_draw_window()
{
    if (!window_visible)
        return;

    if (ImGui::Begin("API Intercept: TerminateProcess", &window_visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
    {
        ImGui::Checkbox("Active", &is_active);
        ImGui::Checkbox("Log intercept", &log_intercept);
        ImGui::NewLine();

        ImGui::RadioButton("Do nothing", &mode, mode_::DO_NOTHING);

        static const char *opt[] = { "Failed", "Success" };
        ImGui::RadioButton("Immediately Return", &mode, mode_::IMMEDIATELY_RETURN);
        ImGui::SameLine();
        ImGui::Combo("##default_return", &default_return, opt, IM_ARRAYSIZE(opt));

        ImGui::RadioButton("Filter by Processs name", &mode, mode_::FILTER_BY_NAME);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Immediately returns successful only for the\nselected process without calling the original API");

        static char name_search_buff[MAX_PATH] = { '\0' };
        ImGui::InputText("##name_search_buff", name_search_buff, MAX_PATH);
        ImGui::SameLine();
        ImGui::Text("(CaseSens)");
        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
            cache_process();

        ImGui::BeginChild("proclist", ImVec2 { 365.f, 200.f }, true);

        for (auto &proc : proc_name_filter)
        {
            if (name_search_buff[0] == '\0' || proc.first.find(name_search_buff) != std::string::npos)
                ImGui::Checkbox(proc.first.c_str(), &proc.second);
        }

        ImGui::EndChild();
    }
    ImGui::End();
}
