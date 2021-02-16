#pragma once

#include <string>
#include <utils/arch.h>

class patchii_module_base
{
public:
	patchii_module_base(std::string name_);
	virtual ~patchii_module_base();

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
};