#include "console.h"

#include <Windows.h>

static HANDLE handle_con_out = nullptr;
static HWND   handle_con_wnd = nullptr;

bool console::initialize()
{
	if (AllocConsole())
	{
		FILE *file_ptr;
		freopen_s(&file_ptr, "CONOUT$", "w", stdout);
		freopen_s(&file_ptr, "CONOUT$", "w", stderr);
		freopen_s(&file_ptr, "CONIN$", "r", stdin);
	}

	handle_con_out = GetStdHandle(STD_OUTPUT_HANDLE);
	handle_con_wnd = GetConsoleWindow();
	return handle_con_out && handle_con_wnd;
}

void *console::get_hwnd()
{
	return handle_con_wnd;
}

void console::print_tf(const char *msg_true, const char *msg_false, bool result)
{
	if (result)
	{
		console::set_color(console::color::LGREEN);
		std::cout << msg_true;
	}
	else
	{
		console::set_color(console::color::LRED);
		std::cout << msg_false;
	}

	console::set_color();
}

void console::set_color(console::color foreground, console::color background)
{
	SetConsoleTextAttribute(handle_con_out, (static_cast<WORD>(background) << 4) | static_cast<WORD>(foreground) );
}

void console::print_error(std::string_view msg)
{
	console::set_color(console::color::LRED);
	std::cout << "\nERROR: ";
	console::set_color();
	std::cout << msg;
}

void console::print_warning(std::string_view msg)
{
	console::set_color(console::color::LYELLOW);
	std::cout << "\nWARNING: ";
	console::set_color();
	std::cout << msg;
}

console::status_print::status_print(std::string_view str)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi{};

	// Get starting point for the current row point before printing, used for printing the current status later
	printf("\n");
	GetConsoleScreenBufferInfo(handle_con_out, &csbi);
	this->status_x = csbi.dwCursorPosition.X;
	this->status_y = csbi.dwCursorPosition.Y;

	// Print the entire string
	console::set_color();
	std::cout << "[      ] " << str;

	// Set status
	this->custom(" WAIT ", console::color::GRAY);
	this->is_called = false; // Reset flag for actual use later
}

console::status_print::status_print(std::wstring_view str)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi{};

	// Get starting point for the current row point before printing, used for printing the current status later
	printf("\n");
	GetConsoleScreenBufferInfo(handle_con_out, &csbi);
	this->status_x = csbi.dwCursorPosition.X;
	this->status_y = csbi.dwCursorPosition.Y;

	// Print the entire string
	console::set_color();
	std::wcout << "[      ] " << str;

	// Set status
	this->custom(" WAIT ", console::color::GRAY);
	this->is_called = false; // Reset flag for actual use later
}

console::status_print::~status_print()
{
	if (this->is_called)
		return;

	this->custom(" LOST ", console::color::GRAY);
}

bool console::status_print::autoset(bool result)
{
	if (result)
		this->ok();
	else
		this->fail();

	return result;
}

void console::status_print::ok()
{
	this->custom("  OK  ", console::color::LGREEN);
}

void console::status_print::fail()
{
	this->custom(" FAIL ", console::color::LRED);
}

void console::status_print::custom(const char status[7], console::color foreground, console::color background)
{
	this->is_called = true;

	CONSOLE_SCREEN_BUFFER_INFO csbi{};

	// Get current cursor position for restoration later
	COORD coord_orig{};
	GetConsoleScreenBufferInfo(handle_con_out, &csbi);
	coord_orig = csbi.dwCursorPosition;

	// Print custom status
	COORD coord_status = { this->status_x + 1, this->status_y };
	SetConsoleCursorPosition(handle_con_out, coord_status);
	console::set_color(foreground, background);
	std::cout << status;

	// Reset console cursor position back to original
	console::set_color();
	SetConsoleCursorPosition(handle_con_out, coord_orig);
}
