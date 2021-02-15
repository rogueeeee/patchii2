#pragma once

#include <string>
#include <stdio.h>
#include <iostream>

namespace console
{
	enum class color : unsigned char
	{
		BLACK   = 0x00,
		BLUE    = 0x01,
		GREEN   = 0x02,
		AQUA    = 0x03,
		RED     = 0x04,
		PURPLE  = 0x05,
		YELLOW  = 0x06,
		WHITE   = 0x07,
		GRAY    = 0x08,
		LBLUE   = 0x09,
		LGREEN  = 0x0A,
		LAQUA   = 0x0B,
		LRED    = 0x0C,
		LPURPLE = 0x0D,
		LYELLOW = 0x0E,
		BWHITE  = 0x0F,
	};

	bool initialize();
	void *get_hwnd();
	void print_tf(const char *msg_true, const char *msg_false, bool result);
	void set_color(console::color foreground = console::color::BWHITE, console::color background = console::color::BLACK);
	void print_error(std::string_view msg);
	void print_warning(std::string_view msg);

	class status_print
	{
	public:
		status_print(std::string_view str);
		status_print(std::wstring_view str);
		~status_print();

	public:
		void autoset(bool result);
		void ok();
		void fail();
		void custom(const char status[7], console::color foreground, console::color background = console::color::BLACK);

	private:
		bool  is_called = false;
		short status_x  = 0;
		short status_y  = 0;
	};
}