#include "ui.h"
#include <client/globals.h>
#include <Windows.h>
#include <utils/console.h>
#include <utils/arch.h>
#include <client/bin_header/patchii_img_128x201.h>

#include <patchii_version.h>

void ui::callbacks::about()
{
	static LPDIRECT3DTEXTURE9 patchii_image = globals::window_instance->make_texture_from_memory(patchii_img_bin, sizeof(patchii_img_bin), 128, 201);

	if (ImGui::Begin("About"))
	{
		ImGui::Image(patchii_image, ImVec2{ 128.f, 201.f });
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("by fantastiic\nhttps://www.deviantart.com/fantastiic/art/Chibi-Patchouli-Knownledge-Touhou-305044472");

		ImGui::SameLine();
		ImGui::Text(
			PATCHII_DESCRIPTION "\n"
			"Arch: " ARCH_STR "\n"
			"\n"
			"Dear IMGui 1.81 - ocornut\n"
			"MinHook 1.3.3 - TsudaKageyu\n"
			"\n"
		);

		ImGui::TextColored(ImVec4 { .67f, .84f, .90f, 1.f }, "Repository: https://github.com/rogueeeee/patchii2");
		if (ImGui::IsItemClicked())
			ShellExecuteW(NULL, L"open", L"https://github.com/rogueeeee/patchii2", nullptr, nullptr, SW_SHOW);	
	}
	ImGui::End();
}
