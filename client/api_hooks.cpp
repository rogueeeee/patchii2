#include "api_hooks.h"

#include <unordered_map>
#include <vector>

static std::unordered_map<const char *, std::vector<void*>> api_hooks;



bool patchii_apihooks_enable()
{
	return true;
}

bool patchii_apihooks_disable()
{
	return true;
}

bool patchii_apihooks_register(const char *api_name, void *callback)
{
	if (api_hooks.find(api_name) == api_hooks.end())
		return false;

	return true;
}

bool patchii_apihooks_unregister(const char *api_name, void *callback)
{
	if (api_hooks.find(api_name) == api_hooks.end())
		return false;


	return true;
}
