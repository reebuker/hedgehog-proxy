#pragma once

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

namespace env 
{
    void set_proxy();
    void unset_proxy();
}
