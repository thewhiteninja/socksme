#include "utils.h"
#include "options.h"

#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <mutex>

std::mutex log_mutex;

namespace utils
{
	bool is_valid_port(uint32_t port)
	{
		return port <= 65535;
	}

	bool is_valid_ip(std::string ip)
	{
		struct sockaddr_in sa;
		struct sockaddr_in6 sa6;
		int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
		if (result == 0)
		{
			return inet_pton(AF_INET6, ip.c_str(), &(sa6.sin6_addr)) != 0;
		}
		return true;
	}

    std::string basename(const std::string& str)
    {
        size_t found = str.find_last_of("/\\");
        if (found == std::string::npos)
        {
            return str;
        }
        else
        {
            return str.substr(found + 1);
        }
    }

    std::string ip_to_str(const struct sockaddr *sa)
    {
        char buf[128] = { 0 };

        switch (sa->sa_family)
        {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), buf, sizeof(buf));
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), buf, sizeof(buf));
            break;

        default:
            return "Unknown AF";
        }

        return std::string(buf);
    }

    uint16_t get_port(const struct sockaddr *sa)
    {
        if (sa->sa_family == AF_INET)
        {
            return (((struct sockaddr_in*)sa)->sin_port);
        }
        return (((struct sockaddr_in6*)sa)->sin6_port);
    }

    std::vector<std::uint8_t> intersection(std::vector<std::uint8_t> &v1, std::vector<std::uint8_t> &v2)
    {
        std::vector<std::uint8_t> v3;

        std::sort(v1.begin(), v1.end());
        std::sort(v2.begin(), v2.end());

        std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v3));
        std::sort(v3.begin(), v3.end());

        return v3;
    }

}

namespace logs
{
	void base(loglevel l, std::string message, int err)
    {
        if (l <= Options::log_level)
        {
            std::ostringstream stream;

            SYSTEMTIME st;
            GetLocalTime(&st);
            char buf[64] = { 0 };
            const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec" };
            sprintf_s(buf, 64, "[%02u %s %04u %02u:%02u:%02u]", st.wDay, months[st.wMonth - 1], st.wYear, st.wHour, st.wMinute, st.wSecond);

            const char* priority_names[] = { "ERR ", "INFO", "DBG " };
            stream << buf << " - " << priority_names[l] << " - " << message;

            if (err != 0)
            {
                stream << " - 0x" << std::setfill('0') << std::hex << err;
            }

            log_mutex.lock();

            std::cout << stream.str() << std::endl;
            std::flush(std::cout);

            log_mutex.unlock();
        }
    }

	void info(std::string message)
    {
        base(loglevel::INFO, message, 0);
    }

	void err(std::string message, int err)
    {
        base(loglevel::ERR, message, err);
    }

	void debug(std::string message, int err)
    {
        base(loglevel::DEBUG, message, err);
    }

}