#include "modules.h"
#include <utils/console.h>

#define patchii_register_module(name) std::make_pair(new (std::nothrow) name##(), #name),

std::vector<patchii_module_base *> patchii_get_registered_modules()
{
	std::vector<std::pair<patchii_module_base *, const char *>> preload =
	{
		// Platform agnostic modules - Place module here if it works for both x86 and x64



		#ifdef WIN32
		// x86 modules - Place module here if it only works for x86



		#else
		// x64 modules - Place module here if it only works for x64



		#endif
	};

	std::vector<patchii_module_base *> loaded;
	for (auto module : preload)
	{
		if (!module.first)
		{
			console::print_error(std::string("Failed to register module: ") + module.second);
			continue;
		}

		loaded.push_back(module.first);
		std::cout << "\nRegistered: " << module.second;
	}

	return loaded;
}