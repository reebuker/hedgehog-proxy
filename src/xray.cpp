#include "xray.h"

#include "constants.h"

namespace xray
{
    void gen_config(const proxy::Config& cfg, const std::string& out_path)
    {
        using namespace nlohmann;
        json config;

        // ---- Basic parameters ----
        config["remarks"] = cfg.remarks;
        config["log"] = {
            {"loglevel", "debug"},
            {"error", "/tmp/hedgehog_xray_error.log"}
        };

        // ---- Policy ----
        json levels = 
        {"levels", {
            {"8", {
                {"connIdle", 300}, 
                {"handshake", 4},
            }}
        }};

        json system = {"system", {}};

        config["policy"] = {levels, system};

        // ---- Inbounds ----
        json socks = {
            {"listen", "127.0.0.1"},
            {"port", Config::SOCKS_PORT},
            {"protocol", "socks"},
            {"settings", {
                {"auth", "noauth"},
                {"udp", true},
                {"userLevel", 8}
            }},
            {"tag", "socks"}
        };
        
        json http = {
            {"listen", "127.0.0.1"},
            {"port", Config::HTTP_PORT},
            {"protocol", "http"},
            {"settings", {
                {"userLevel", 8}
            }},
            {"tag", "http"}
        };

        config["inbounds"] = {socks, http/*, dokodemo*/};

        // Первый outbound
        json proxy_out = {
            {"protocol", cfg.protocol},
            {"tag", "proxy_out"},
            {"streamSettings", {
                // Тип транспорта, по умолчанию TCP
                {"network", cfg.params.count("type") ? cfg.params.at("type") : "tcp"}
            }}
        };

        json server;
        server["address"] = cfg.address;
        server["port"]    = cfg.port;

        if (cfg.protocol == "trojan") {
            server["password"] = cfg.uuid;
            server["level"]    = 1;
            // server["method"]   = "chacha20";
            // server["ota"]      = false;
            proxy_out["settings"]["servers"] = json::array({server});
        }
        else {
            server["users"] = json::array({{
                {"id",         cfg.uuid},
                {"encryption", "none"},
                {"level",      8},
                {"security",   "auto"}
            }});
            proxy_out["settings"]["vnext"] = json::array({server});
        }

        // ---- Security parameter ----
        std::string security = cfg.params.count("security") ? cfg.params.at("security") : "none";
        proxy_out["streamSettings"]["security"] = security;
        if (security == "reality") {
            // flow parameter
            if (cfg.params.count("flow")) {
                proxy_out["settings"]["vnext"][0]["users"][0]["flow"] = cfg.params.at("flow");
            }

            proxy_out["streamSettings"]["realitySettings"] = {
                {"show", false},
                {"publicKey", cfg.params.count("pbk") ? cfg.params.at("pbk") : ""},
                {"shortId", cfg.params.count("sid") ? cfg.params.at("sid") : ""},
                {"fingerprint", cfg.params.count("fp") ? cfg.params.at("fp") : "chrome"},
                {"serverName", cfg.params.count("sni") ? cfg.params.at("sni") : ""},
            };
        }
        else if (security == "tls") {
            proxy_out["streamSettings"]["tlsSettings"] = {
                {"serverName", cfg.params.count("sni") ? cfg.params.at("sni") : ""},
                {"allowInsecure", false},
                {"show", false},
                {"fingerprint", "chrome"}
            };
        }

        std::string transport = cfg.params.count("type") ? cfg.params.at("type") : "tcp";
        // ---- WebSocket ----
        if (transport == "ws") {
            std::string host_value;
            if (cfg.params.count("host") && !cfg.params.at("host").empty()) {
                host_value = cfg.params.at("host");
            }
            else {
                host_value = cfg.address;
            }

            proxy_out["streamSettings"]["wsSettings"] = {
                {"path", cfg.params.count("path") ? cfg.params.at("path") : "/"},
                {"headers", {
                    {"Host", host_value}
            }}};
        }
        
        // ---- gRPC ----
        if (transport == "grpc") {
            proxy_out["streamSettings"]["grpcSettings"] = {
                {"serviceName", cfg.params.count("serviceName") ? cfg.params.at("serviceName") : ""},
                {"health_check_timeout", 20},
                {"idle_timeout", 60},
                {"multiMode", true},
                {"permit_without_stream", false},
                {"authority", ""}
            };
        }
    
        // ---- General parameters (should be last) ----
        // fragment parameter
        if (cfg.fragment.enabled) {
            proxy_out["streamSettings"]["sockopt"]["fragment"] = {
                {"length", cfg.fragment.length},
                {"interval", cfg.fragment.interval},
                {"packets", cfg.fragment.packets}
            };
        }
    
        // mux parameter
        int mux_concurrency = cfg.params.count("muxConcurrency") ? std::stoi(cfg.params.at("muxConcurrency")) : -1;
        bool mux_enabled = (cfg.params.count("mux") && cfg.params.at("mux") == "true");
        proxy_out["mux"] = {
            {"enabled", mux_enabled},
            {"concurrency", mux_concurrency},
            {"xudpConcurrency", 8},
            {"xudpProxyUDP443", ""}
        };

        // alpn parameter 
        if (!cfg.alpn.empty()) {
            proxy_out["streamSettings"]["tlsSettings"]["alpn"] = cfg.alpn;
        }
        
        // fingerprint parameter
        if (cfg.params.count("fp")) {
            proxy_out["streamSettings"]["tlsSettings"]["fingerprint"] = cfg.params.at("fp");
        }
        
        // Второй outbound
        json direct_out = {
            {"protocol", "freedom"},
            {"tag", "direct_out"}
        };

        config["outbounds"] = {proxy_out, direct_out};

        // TODO: Маршрутизация, чтобы обходил git, pacman, yay, pip и тд

        std::ofstream file(out_path);
        if (file.is_open()) {
            file << config.dump(2);
            file.close();
        }
        else {
            std::cerr << "Failed to create/open output file: " << out_path << "\n";
        }
    }

    pid_t start(const std::string& config_path) 
    {
        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Failed to create child process\n";
            return -1;
        }
        else if (pid == 0) {
            char* argv[] = {
                (char*)"xray",
                (char*)"-c",
                (char*)config_path.c_str(),
                nullptr
            };

            // if successfully executes, will never return to og program
            execvp("xray", argv);

            // if failed to execvp
            exit(1);
        }

        std::ofstream file(Config::PID_FILE_PATH);
        if (file.is_open()) {
            file << pid;
            file.close();
        }
        else {
            std::cerr << "Failed to write pid to file: " << Config::PID_FILE_PATH << "\n";
        }

        return pid; 
    }

    void stop()
    {
        pid_t saved_pid;

        std::ifstream file(Config::PID_FILE_PATH);
        if (!file.is_open()) {
            std::cerr << "Failed to read pid from file: " << Config::PID_FILE_PATH << "\n";
            return;
        }

        file >> saved_pid;
        file.close();

        kill(saved_pid, SIGTERM);
        std::remove(Config::PID_FILE_PATH);
    }

    bool is_running()
    {
        pid_t saved_pid;
        std::ifstream file(Config::PID_FILE_PATH);
        if (!file.is_open()) {
            return false;
        }
        file >> saved_pid;
        file.close();

        if (kill(saved_pid, 0) == 0) {
            return true;
        }
        else {
            std::remove(Config::PID_FILE_PATH);
        }

        return false;
    }
}
