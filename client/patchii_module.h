#pragma once

#include <string>
#include <utils/arch.h>

class patchii_module
{
public:
	patchii_module(std::string name_, arch target_arch_);
	virtual ~patchii_module();

public:
	virtual bool load() { return false; };
	virtual bool unload() { return false; };
	virtual bool is_loaded() { return false; };

	virtual void draw_imgui_tools() {};
	virtual void draw_imgui_mainmenubar() {};
	virtual void draw_imgui_module_options() {};
	virtual void draw_imgui() {};

	virtual void update() {};

public:
	std::string name;
	const arch  target_arch;
};