#include "utils.h"
#include "options.h"
#include "socks_server.h"


int main(int argc, const char** argv)
{
    if (!Options::read_from_args(argc, argv))
    {
        Options::show_usage();
    }
    else
    {
        Options::hello();

        std::unique_ptr<SocksServer> ss = std::make_unique<SocksServer>();
		if (ss->is_ready())
		{
			ss->run();
		}
    }

    return 0;
}

