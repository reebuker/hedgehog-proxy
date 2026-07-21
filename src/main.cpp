#include <iostream>

#include "proxy.h"
#include "xray.h"
#include "env.h"
#include "client.h"
#include "constants.h"

void debug_output()
{
    std::string url;
    std::getline(std::cin, url);

    proxy::Config cfg = proxy::parse_config(url);

    std::cout << "Protocol: " << cfg.protocol << "\n";
    std::cout << "UUID: " << cfg.uuid << "\n";
    std::cout << "Address: " << cfg.address << "\n";
    std::cout << "Port: " << cfg.port << "\n";

    for (size_t i = 0; i < cfg.alpn.size(); i++) {
        std::cout << "alpn[" << i << "]: " << cfg.alpn[i] << "\n";
    }
    std::cout << "Fragment.enabled: " << cfg.fragment.enabled << "\n";
    std::cout << "Fragment.length: " << cfg.fragment.length << "\n";
    std::cout << "Fragment.interval: " << cfg.fragment.interval << "\n";
    std::cout << "Fragment.packets: " << cfg.fragment.packets << "\n";

    std::cout << cfg.remarks << "\n";

    xray::gen_config(cfg, "/home/reebuker/coding/hedgehog/config1.json");
    std::cout << "PID: " << xray::start("/home/reebuker/coding/hedgehog/config1.json") << "\n";
    std::cout << "Is running: " << xray::is_running() << "\n";
    xray::stop();
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        std::cout << "Usage: hedgehog [toggle|load|fetch|status|help]\n";
        return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "toggle") { 
        return client::toggle(); 
    }
    else if (cmd == "load") { 
        if (argc < 3) {
            std::cerr << "No url was passed to the function \n";
            std::cout << "Use hedgehog load protocol://url\n";
            return 1;
        }
        return client::load(argv[2]);
    }
    else if (cmd == "fetch") {
        if (argc < 3) {
            std::cerr << "No url was passed to the function \n";
            std::cout << "Use hedgehog load https://url\n";
            return 1;
        }
        return client::fetch(argv[2]);

    }
    else if (cmd == "status") {
        client::status();
    }
    else if (cmd == "help") {
        std::cout << "Usage: hedgehog [on|off|load|fetch|help]\n";
    }
    else {
        std::cout << "Unknown command: " << cmd << "\n Use 'hedgehog help' \n"; 
    }
    
    return 0;
}
