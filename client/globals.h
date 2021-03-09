#pragma once

#include <cstdint>

namespace globals
{
	inline union dll_
	{
		void         *handle { nullptr };
		std::uint8_t *base;
	} dll;
}