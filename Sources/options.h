#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "utils.h"

class Options
{
private:
	static std::string program_name;
public:
    static std::string bind_ip;
    static uint32_t    bind_port;
	static uint32_t    max_conn;

	static loglevel    log_level;

	static bool read_from_args(int argc, const char** argv);

    static void hello();

	static void show_usage();
};





