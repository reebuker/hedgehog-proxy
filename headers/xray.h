#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <signal.h>

#include "proxy.h"
#include "json.hpp"

namespace xray 
{
    void gen_config(const proxy::Config& cfg, const std::string& out_path);
    pid_t start(const std::string& config_path);
    void stop();
    bool is_running();
}
