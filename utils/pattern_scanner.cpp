#include "pattern_scanner.h"
#include <cstring>

std::uint8_t *pattern_scan(void *start, std::size_t size, const char *pattern, const char *mask)
{
	char               *current        = reinterpret_cast<char *>(start);
	const void         *end            = current + size;
	const std::size_t  pattern_length = strlen(mask) + 1;

	do
	{
		for (std::size_t idx = 0; idx <= pattern_length; idx++)
		{
			if (mask[idx] == '\0')
				return reinterpret_cast<std::uint8_t *>(current);

			if (mask[idx] == '?')
				continue;

			if (current[idx] != pattern[idx])
				break;
		}
	} while (++current + pattern_length <= end);

	return nullptr;
}
