#include "threadmanager.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <imgui.h>
#include <vector>
#include <string>
#include <console.h>

struct thread_cache_t
{
	DWORD id;
	std::string id_str;
	bool suspended;
};

static std::vector<thread_cache_t> cached_threads;
static DWORD client_thread_id = 0;
static bool window_visible = false;
static bool auto_refresh = false;

constexpr ULONGLONG tick_delta = 2000;
static ULONGLONG next_tick_update = 0;

void cache_threads()
{
	HANDLE thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	THREADENTRY32 thread_entry = { sizeof(THREADENTRY32) };
	std::vector<thread_cache_t> temp_thread_entries;

	while (Thread32Next(thread_snap, &thread_entry))
	{
		if (static DWORD client_proc_id = GetCurrentProcessId(); thread_entry.th32OwnerProcessID != client_proc_id)
			continue;
		
		// If it was already cached, just copy it. Should also remove threads that no longer exist
		for (const thread_cache_t &cached_thread : cached_threads)
		{
			if (cached_thread.id == thread_entry.th32ThreadID)
			{
				temp_thread_entries.push_back(cached_thread);
				goto LBL_NEXT_THREAD;
			}
		}

		temp_thread_entries.push_back({
			thread_entry.th32ThreadID,
			std::to_string(thread_entry.th32ThreadID),
			false
		});

		LBL_NEXT_THREAD:;
	}
	
	cached_threads.clear();
	cached_threads = temp_thread_entries;

	CloseHandle(thread_snap);
}

void suspend_thread(thread_cache_t &thread)
{
	console::status_print stat_susp("Suspending thread ID: " + thread.id_str);

	HANDLE open_thread = OpenThread(THREAD_ALL_ACCESS, false, thread.id);
	if (!open_thread)
	{
		stat_susp.fail();
		return;
	}
	
	if (SuspendThread(open_thread) == static_cast<DWORD>(-1))
	{
		stat_susp.fail();
		CloseHandle(open_thread);
		return;
	}

	thread.suspended = true;
	stat_susp.ok();
	CloseHandle(open_thread);
	return;
}

void resume_thread(thread_cache_t &thread)
{
	console::status_print stat_res("Resuming thread ID: " + thread.id_str);

	HANDLE open_thread = OpenThread(THREAD_ALL_ACCESS, false, thread.id);
	if (!open_thread)
	{
		stat_res.fail();
		return;
	}

	if (ResumeThread(open_thread) == static_cast<DWORD>(-1))
	{
		stat_res.fail();
		CloseHandle(open_thread);
		return;
	}

	thread.suspended = false;
	stat_res.ok();
	CloseHandle(open_thread);
	return;
}

bool threadmanager_load()
{
	client_thread_id = GetCurrentThreadId();
	return true;
}

bool threadmanager_unload()
{
	client_thread_id = 0;
	window_visible = false;
	auto_refresh = false;
	next_tick_update = 0;

	cached_threads.clear();
	return true;
}

void threadmanager_toggle()
{
	window_visible = !window_visible;
}

void threadmanager_draw_window()
{
	if (!window_visible)
		return;

	if (auto_refresh)
	{
		ULONGLONG current_tick = GetTickCount64();
		if (current_tick >= next_tick_update)
		{
			next_tick_update = current_tick + tick_delta;
			cache_threads();
		}
	}

	if (ImGui::Begin("Thread Manager", &window_visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Checkbox("Auto", &auto_refresh);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Automatically refresh the thread list");

		ImGui::SameLine();
		if (ImGui::Button("Refresh"))
			cache_threads();

		ImGui::SameLine();
		if (ImGui::Button("Suspend All"))
		{
			for (auto &thread : cached_threads)
				if (!thread.suspended && thread.id != client_thread_id)
					suspend_thread(thread);
		}

		ImGui::SameLine();
		if (ImGui::Button("Unsuspend All"))
		{
			for (auto &thread : cached_threads)
				if (thread.suspended && thread.id != client_thread_id)
					resume_thread(thread);
		}
		
		ImGui::BeginChild("threadidlist", ImVec2 { 316.f, 350.f }, true);
		for (std::size_t idx = 0; idx < cached_threads.size(); idx++)
		{
			thread_cache_t &cached_thread = cached_threads[idx];
		
			const char *button_text = "No Action";
			ImVec4 text_color = ImVec4 { 1.f, 1.f, 0.f, 1.f };

			if (cached_thread.id != client_thread_id)
			{
				button_text = cached_thread.suspended ? "Unsuspend" : " Suspend ";
				text_color  = cached_thread.suspended ? ImVec4{ 1.f, 0.f, 0.f, 1.f } : ImVec4{ 0.f, 1.f, 0.f, 1.f };
			}
		
			ImGui::PushID(idx);
			if (ImGui::Button(button_text) && cached_thread.id != client_thread_id)
			{
				std::cout << "\n2";

				if (cached_thread.suspended)
					resume_thread(cached_thread);
				else
					suspend_thread(cached_thread);
			}
			ImGui::PopID();

			ImGui::SameLine();
			ImGui::Text("ID:");
			ImGui::SameLine();
			ImGui::TextColored(text_color, cached_thread.id_str.c_str());
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
