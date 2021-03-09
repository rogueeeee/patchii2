#pragma once

#include <cstdint>

namespace globals
{
	static union
	{
		void         *handle { nullptr };
		std::uint8_t *base;
	} dll;
}