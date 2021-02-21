#pragma once

#include <client/patchii_module_base.h>

class module_threadman : public patchii_module_base
{
public:
	module_threadman();

	virtual bool load() override;
	virtual bool unload() override;
	virtual bool is_loaded() override;
	virtual void draw_imgui_tools() override;
	virtual void draw_imgui() override;
	virtual void update() override;
};