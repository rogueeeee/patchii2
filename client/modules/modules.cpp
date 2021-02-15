#include "modules.h"
#include <utils/console.h>

#define patchii_register_module(name) std::make_pair(new (std::nothrow) name##(), #name)

std::vector<patchii_module *> patchii_get_registered_modules()
{
	std::vector<std::pair<patchii_module *, const char *>> preload =
	{
		// Place modules to preload here using the patchii_register_module macro
	};

	std::vector<patchii_module *> loaded;
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