#pragma once

#include <client/patchii_imgui_window.h>

namespace ui
{
	namespace callbacks
	{
		void about();
	}

	inline patchii_imguiwindow about(callbacks::about);
}