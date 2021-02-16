#include "patchii_module_base.h"

patchii_module_base::patchii_module_base(std::string name_)
	: name(name_)
{
}

patchii_module_base::~patchii_module_base()
{
	this->unload();
}
