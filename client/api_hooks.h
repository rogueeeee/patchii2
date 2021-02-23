#pragma once

#include <cstddef>
#include <cstdint>

struct api_hook_flags
{
	enum
	{
		CONTINUE           = 0,
		END                = (1 << 0),
		DONT_CALL_ORIGINAL = (1 << 1),
		USE_EVENT_RETURN   = (1 << 2),
	};
};

struct api_hook_event
{
	void *return_address { 0 };
	union
	{
		void          *ptr;
		std::int8_t    i8;
		std::uint8_t   u8;
		std::int16_t   i16;
		std::uint16_t  u16;
		std::int32_t   i32;
		std::uint32_t  u32;
		std::int64_t   i64;
		std::uint64_t  u64;
	} ret_val { 0 };

	std::uint32_t flags { 0 };
};

bool patchii_apihooks_enable();
bool patchii_apihooks_disable();

bool patchii_apihooks_register(const char *api_name, void *callback);
bool patchii_apihooks_unregister(const char *api_name, void *callback);