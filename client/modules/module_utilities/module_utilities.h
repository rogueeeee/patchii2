#pragma once

#include <client/patchii_module_base.h>

class module_utilities : public patchii_module_base
{
public:
	module_utilities();

	virtual bool load() override;
	virtual bool unload() override;
	virtual bool is_loaded() override;
	virtual void draw_imgui_tools() override;
	virtual void draw_imgui_module_options() override;
	virtual void draw_imgui() override;
};