#pragma once

#include <map>
#include <string>
#include <sstream>
#include <vector>

static std::string symbols = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

namespace proxy 
{
    struct FragmentConfig 
    {
        bool enabled = false;
        std::string length = "100-200";
        std::string interval = "10-20";
        std::string packets = "tlshello";
    };
    
    struct Config 
    {
        std::map<std::string, std::string> params;
        std::vector<std::string> alpn;
        FragmentConfig fragment;

        std::string protocol;
        std::string uuid;
        std::string address;
        std::string remarks;

        int port;
    };

    std::string base64_decode(const std::string_view input);

    std::vector<std::string> split_lines(const std::string& text);

    std::string url_decode(std::string_view short_url);

    FragmentConfig parse_fragment(std::string_view fragment);
    std::vector<std::string> parse_alpn(std::string_view alpn);

    std::string sanitize_remark(const Config& cfg);

    Config parse_config(std::string_view url);
}


