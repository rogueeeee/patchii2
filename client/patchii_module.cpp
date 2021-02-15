#include "patchii_module.h"

patchii_module::patchii_module(std::string name_, arch target_arch_)
	: name(name_), target_arch(target_arch_)
{
}

patchii_module::~patchii_module()
{
	this->unload();
}
