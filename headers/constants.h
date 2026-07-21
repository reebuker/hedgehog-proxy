#pragma once

#include <filesystem>
#include <cstdlib>

namespace Config {
    // Порты
    inline constexpr int SOCKS_PORT = 10808;
    inline constexpr int HTTP_PORT = 10809;
    // inline constexpr int DOKODEMO_PORT = 11111;

    // ---- Временные файлы на RAM ---- 
    inline constexpr const char* TEMP_FETCH_PATH = "/tmp/hedgehog_sub.txt";
    inline constexpr const char* RUN_CONFIG_PATH = "/tmp/hedgehog-run.json";
    inline constexpr const char* PID_FILE_PATH = "/tmp/hedgehog.pid";

    // ---- Постоянные файлы на диске ---- 
    // base dir ~/.config/hedgehog/
    inline std::filesystem::path get_base_dir() {
        const char* home = std::getenv("HOME");
        if (!home) { return "/tmp/hedgehog"; }
        return std::filesystem::path(home) / ".config" / "hedgehog" ;
    }

    // active config path ~/.config/hedgehog/active.json
    inline std::filesystem::path get_active_config_path() {
        return get_base_dir() / "active.json" ;
    }

    // configs dir ~/.config/hedgehog/configs/
    inline std::filesystem::path get_configs_dir() {
        return get_base_dir() / "configs" ;
    }
}
