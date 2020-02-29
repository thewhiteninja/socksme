#include "options.h"
#include "utils.h"
#include "version.h"

#include <iostream>
#include <ctime>

///////////////////////////////////////////////////////////////////////////////

std::string Options::program_name = "socksme.exe";

std::string Options::bind_ip	   = "0.0.0.0";
uint32_t    Options::bind_port = 8888;
uint32_t    Options::max_conn  = 32;
loglevel    Options::log_level = loglevel::INFO;

///////////////////////////////////////////////////////////////////////////////

bool Options::read_from_args(int argc, const char** argv)
{
	Options::program_name = std::string(argv[0]);

	int i = 1;
	while (i < argc)
    {
        if (!strncmp(argv[i], "-h", 2) || !strncmp(argv[i], "--help", 6))
        {
            return false;
        }
        if (!strncmp(argv[i], "-s", 2) || !strncmp(argv[1], "--source-addr", 13))
        {
            if (i + 1 >= argc) return false;

            Options::bind_ip = argv[++i];
			if (!utils::is_valid_ip(Options::bind_ip)) return false;
			continue;
        }
        if (!strncmp(argv[i], "-p", 2) || !strncmp(argv[i], "--source-port", 13))
        {
            if (i + 1 >= argc) return false;

            Options::bind_port = atoi(argv[++i]);
			if (!utils::is_valid_port(Options::bind_port)) return false;
            continue;
        }
		if (!strncmp(argv[i], "-n", 2) || !strncmp(argv[i], "--max-conn", 10))
        {
            if (i + 1 >= argc) return false;

            Options::max_conn = atoi(argv[++i]);
			if ((1 > Options::max_conn) || (Options::max_conn > 64)) return false;
            continue;
        }
		if (!strncmp(argv[i], "-v", 2) || !strncmp(argv[i], "--log-level", 11))
		{
            if (i + 1 >= argc) return false;

            int level = atoi(argv[++i]);
			switch(level)
			{
			case 0:
				Options::log_level = loglevel::ERR;
				break;
			case 1:
				Options::log_level = loglevel::INFO;
				break;
			case 2:
				Options::log_level = loglevel::DEBUG;
				break;
			default:
				return false;
			}		
		}
        i++;
    }
    return true;
}

void Options::show_usage()
{
    std::cout << "Usage : " << program_name << " [options]" << std::endl << std::endl;
    std::cout << "    Options :" << std::endl;
	std::cout << "        -h, --help : show this message" << std::endl;
	std::cout << std::endl;
    std::cout << "        -s, --source-addr : set local ip address    (0.0.0.0)" << std::endl;
    std::cout << "        -p, --source-port : set local port          (8888) " << std::endl;
	std::cout << "        -b, --backlog     : set backlog value       (5)" << std::endl;
    std::cout << std::endl;
	std::cout << "        -v, --log-level   : set logging level [0-2] (1)" << std::endl;
	std::cout << std::endl;
    exit(1);
}

void Options::hello()
{
    time_t t = time(NULL);
    char timestr[64] = { 0 };
    ctime_s(timestr, sizeof(timestr), &t);
    timestr[strlen(timestr) - 1] = '\0';
    std::cout << "Starting " << Options::program_name << " at " << timestr << " (version " << VERSION << ")" << std::endl << std::endl;
}