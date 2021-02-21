#include "modules.h"
#include <console.h>



// Platform agnostic modules - Place module here if it works for both x86 and x64
#include "module_threadman/module_threadman.h"

#ifdef _M_IX86 // x86 modules - Place module here if it only works for x86
#elif _M_X64   // x64 modules - Place module here if it only works for x64
#endif

#define patchii_register_module(name) std::make_pair(new (std::nothrow) name##(), #name),

void patchii_get_registered_modules(std::vector<patchii_module_base *> &dest)
{
	std::vector<std::pair<patchii_module_base *, const char *>> preload =
	{
		// Platform agnostic modules - Place module here if it works for both x86 and x64
		patchii_register_module(module_threadman)

		#ifdef _M_IX86 // x86 modules - Place module here if it only works for x86
		#elif _M_X64   // x64 modules - Place module here if it only works for x64
		#endif
	};
	
	for (auto module : preload)
	{
		if (!module.first)
		{
			console::print_error(std::string("Failed to register module: ") + module.second);
			continue;
		}
		
		dest.push_back(module.first);
		std::cout << "\nRegistered: " << module.second;
	}
}