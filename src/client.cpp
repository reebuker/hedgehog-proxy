#include "client.h"

#include "constants.h"
#include "proxy.h"
#include "xray.h"
#include "env.h"

namespace client
{
    int toggle()
    {
        // turn on
        if (!xray::is_running()) {
            if (!std::filesystem::exists(Config::get_active_config_path())) {
                std::cerr << "No active proxy selected. Select one first\n";
                return 1;
            }

            std::filesystem::copy_file(Config::get_active_config_path(), 
                                        Config::RUN_CONFIG_PATH,
                                        std::filesystem::copy_options::overwrite_existing);

            xray::start(Config::RUN_CONFIG_PATH);
            env::set_proxy();

            std::cout << "Proxy successfully started!\n";
        }
        // turn off
        else {
            xray::stop();
            env::unset_proxy();
            std::cout << "Proxy is stopped. System proxy settings cleared\n";
        }

        return 0;
    }

    int load(const char* url)
    {
        proxy::Config cfg = proxy::parse_config(url);

        if (cfg.protocol.empty()) {
            std::cerr << "Failed to parse proxy. Invalid protocol\n";
            return 1;
        }

        std::filesystem::path active_path = Config::get_active_config_path();
        std::filesystem::create_directories(active_path.parent_path());

        xray::gen_config(cfg, active_path);
        std::cout << "Successfully loaded and activated config\n";

        return 0;
    }

    int fetch(const char* subs_url)
    {
        // ---- Downloading and reading data ----
        char curl_cmd[2048];
        std::cout << "DEBUG URL: " << subs_url << "\n";
        std::cout << "DEBUG PATH: " << Config::TEMP_FETCH_PATH << "\n";

        int written = snprintf(curl_cmd, sizeof(curl_cmd),
                            "curl -s -L \"%s\" -o %s",
                            subs_url, Config::TEMP_FETCH_PATH);

        if (written >= static_cast<int>(sizeof(curl_cmd))) {
            std::cerr << "Subscription url was too long\n";
            return 1;
        }

        int res = std::system(curl_cmd);
        if (res != 0) {
            std::cerr << "Failed to download subscription file via curl\n";
            return 1;
        }

        FILE* file = std::fopen(Config::TEMP_FETCH_PATH, "r");
        if (!file) {
            std::cerr << "Failed to open downloaded text file\n";
        }
        
        char raw_buf[32768];

        if (std::fgets(raw_buf, sizeof(raw_buf), file) == nullptr) {
            std::fclose(file);
            std::remove(Config::TEMP_FETCH_PATH);
            std::cerr << "Downloaded subscription file is empty\n";
            return 1;
        }

        std::fclose(file);
        std::remove(Config::TEMP_FETCH_PATH);

        // ---- Data parsing ----
        std::string content(raw_buf);
        std::string urls = proxy::base64_decode(content);

        std::vector<std::string> proxy_links = proxy::split_lines(urls);
        if (proxy_links.empty()) {
            std::cerr << "No proxy links was found in the subscription\n";
            return 1;
        }

        std::filesystem::path configs_dir = Config::get_configs_dir();
        std::filesystem::create_directories(configs_dir);

        int index = 1;
        for (const std::string& link : proxy_links) {
            proxy::Config cfg = proxy::parse_config(link);
            if (cfg.protocol.empty()) continue;

            std::string safe_name = proxy::sanitize_remark(cfg);
            std::string filename = safe_name + "_" + std::to_string(index) + ".json";
            std::filesystem::path out_file = configs_dir / filename;

            xray::gen_config(cfg, out_file.string());
            std::cout << " [" << index << "] Saved: " << filename << "\n";
            index++;
        }

        return 0;
    }

    void status() 
    {
        bool running = xray::is_running();
        std::cout << "{"
            << "\"text\": \"" << (running ? "󰌆 VPN" : "󰌊 VPN") << "\","
            << "\"class\": \"" << (running ? "hedgehog-on" : "hedgehog-off") << "\","
            << "\"tooltip\": \"" << (running ? "hedgehog is active" : "hedgehog is turned off") << "\","
            << "}\n";
    }
}
