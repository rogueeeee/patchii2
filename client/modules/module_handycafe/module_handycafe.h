#pragma once

#include <client/patchii_module_base.h>

class module_handycafe : public patchii_module_base
{
public:
	module_handycafe();

	virtual bool load() override;
	virtual bool unload() override;
	virtual bool is_loaded() override;
	
	virtual void draw_imgui_mainmenubar() override;
	virtual void draw_imgui() override;
};