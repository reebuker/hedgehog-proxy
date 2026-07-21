#include "env.h"
#include "constants.h"

#include <iostream>
#include <fstream>

namespace env
{
    void set_proxy()
    {
        char cmd[512];
        std::snprintf(cmd, sizeof(cmd),
            "gsettings set org.gnome.system.proxy mode 'manual' && "
            "gsettings set org.gnome.system.proxy.http host '127.0.0.1' && "
            "gsettings set org.gnome.system.proxy.http port %d && "
            "gsettings set org.gnome.system.proxy.https host '127.0.0.1' && "
            "gsettings set org.gnome.system.proxy.https port %d && "
            "gsettings set org.gnome.system.proxy.socks host '127.0.0.1' && "
            "gsettings set org.gnome.system.proxy.socks port %d",
            Config::HTTP_PORT,
            Config::HTTP_PORT,
            Config::SOCKS_PORT
        );

        std::system(cmd);
    }

    void unset_proxy()
    {
        std::system("gsettings set org.gnome.system.proxy mode 'none'");
    }
}
