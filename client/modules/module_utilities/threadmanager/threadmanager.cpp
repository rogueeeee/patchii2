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

// RAII implementation
class thread_open_handle_helper
{
public:
	thread_open_handle_helper(DWORD id)
	{
		this->hnd = OpenThread(THREAD_ALL_ACCESS, false, id);
	}

	~thread_open_handle_helper()
	{
		if (this->hnd)
			CloseHandle(this->hnd);
	}

	explicit operator bool() const
	{
		return hnd;
	}

	const HANDLE &get()
	{
		return hnd;
	}

private:
	HANDLE hnd;
};

static std::vector<thread_cache_t> cached_threads;
static DWORD client_thread_id = 0;
static bool window_visible    = false;
static bool auto_refresh      = false;

constexpr ULONGLONG tick_delta       = 800;
static    ULONGLONG next_tick_update = 0;

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
	if (thread_open_handle_helper handle(thread.id); console::status_print("Suspending thread ID: " + thread.id_str).autoset(handle && SuspendThread(handle.get()) != static_cast<DWORD>(-1)))
		thread.suspended = true;

	return;
}

void resume_thread(thread_cache_t &thread)
{
	if (thread_open_handle_helper handle(thread.id); console::status_print("Resuming thread ID: " + thread.id_str).autoset(handle && ResumeThread(handle.get()) != static_cast<DWORD>(-1)))
		thread.suspended = false;

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
		if (ImGui::Button("Resume All"))
		{
			for (auto &thread : cached_threads)
				if (thread.suspended && thread.id != client_thread_id)
					resume_thread(thread);
		}
		
		ImGui::BeginChild("threadidlist", ImVec2 { 316.f, 350.f }, true);
		for (std::size_t idx = 0; idx < cached_threads.size(); idx++)
		{
			thread_cache_t &thread = cached_threads[idx];
		
			const char *button_text = "Disabled";
			ImVec4 text_color = ImVec4 { 1.f, 1.f, 0.f, 1.f };
			bool is_not_client_thread = thread.id != client_thread_id;

			if (is_not_client_thread)
			{
				button_text = thread.suspended ? "Resume" : "Suspend";
				text_color  = thread.suspended ? ImVec4{ 1.f, 0.f, 0.f, 1.f } : ImVec4{ 0.f, 1.f, 0.f, 1.f };
			}

			ImGui::PushID(idx);
			if (ImGui::Button(is_not_client_thread ? "Terminate" : "Disabled", ImVec2 { 71.f, 20.f }) && is_not_client_thread)
			{
				thread_open_handle_helper handle(thread.id);
				#pragma warning (disable: 6258)
				console::status_print("Terminating thread ID: " + thread.id_str).autoset(handle && TerminateThread(handle.get(), 0));
				#pragma warning (default: 6258)
			}
			ImGui::SameLine();
			
			if (ImGui::Button(button_text, ImVec2 { 71.f, 20.f }) && is_not_client_thread)
			{
				thread.suspended ? resume_thread(thread) : suspend_thread(thread);
			}
			ImGui::PopID();

			ImGui::SameLine();
			ImGui::Text("ID:");
			ImGui::SameLine();
			ImGui::TextColored(text_color, thread.id_str.c_str());
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
