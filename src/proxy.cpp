#include "proxy.h"

#include <stdexcept>
#include <iostream>

namespace proxy 
{
    std::string base64_decode(const std::string_view input)
    {
        std::string output;
        std::vector T(256, -1);
        for (int i = 0; i < 64; i++) {
            T[symbols[i]] = i;    
        }

        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (T[c] == -1) continue;
            val = (val << 6) | T[c];
            valb += 6;
            if (valb >= 0) {
                output.push_back(char(val >> valb));
                valb -= 8;
            }
        }

        return output;
    }

    std::vector<std::string> split_lines(const std::string& text)
    {
        std::vector<std::string> lines;
        std::stringstream ss(text);
        std::string line;

        while (std::getline(ss, line, '\n')) 
        {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (!line.empty()) {
                lines.push_back(line);
            }
        }

        return lines;
    }

    std::string url_decode(std::string_view short_url)
    {
        std::string decoded;
        while (!short_url.empty()) {
            size_t pos = short_url.find('%');
            if (pos != std::string_view::npos && (pos + 2 < short_url.size())) 
            {
                // part before %FF
                std::string_view as_it = short_url.substr(0, pos);
                short_url = short_url.substr(pos + 1);

                decoded.append(std::string(as_it));

                // hex (FF)
                std::string hex_str = std::string(short_url.substr(0, 2));
                short_url = short_url.substr(2);
                int val = std::stoi(hex_str, nullptr, 16);
                char hex = static_cast<char>(val);

                decoded.append(1, hex);
            }
            else 
            {
                decoded.append(std::string(short_url));
                break;
            }
        }
        return decoded;
    }

    FragmentConfig parse_fragment(std::string_view fragment)
    {
        FragmentConfig cfg;
        cfg.enabled = true;

        std::string parameter;

        // Обработка всех крайних случаев "100-200" "100-200,10-20" "100-200,,tlshello" "tlshello"
        size_t pos = fragment.find(',');   
        if (pos != std::string_view::npos) {
            parameter = std::string(fragment.substr(0, pos));
            fragment = fragment.substr(pos + 1);
            if (!parameter.empty()) { cfg.length = parameter; }
        }
        else if (fragment == "tlshello") {
            cfg.packets = parameter;
            return cfg;
        }
        else {
            cfg.length = fragment;
            return cfg;
        }

        pos = fragment.find(',');   
        if (pos != std::string_view::npos) {
            parameter = std::string(fragment.substr(0, pos));
            fragment = fragment.substr(pos + 1);
            if (!parameter.empty()) { cfg.interval = parameter; }
        }
        else {
            cfg.interval = fragment;
            return cfg;
        }

        if (!fragment.empty()) { cfg.packets = fragment; }
        return cfg;
    }

    std::vector<std::string> parse_alpn(std::string_view alpn)
    {
        size_t pos = alpn.find(',');
        std::vector<std::string> items;
        while (pos != std::string_view::npos) {
            items.push_back(std::string(alpn.substr(0, pos)));
            alpn = alpn.substr(pos + 1);
            pos = alpn.find(',');
        }

        if (!alpn.empty()) {
            items.push_back(std::string(alpn));
        }

        return items;
    }

    std::string sanitize_remark(const Config& cfg)
    {
        std::string clean_name;
        clean_name.reserve(cfg.remarks.size());

        for (char c : cfg.remarks) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || (c == '_' && c == '-')) 
            {
                clean_name.push_back(c); 
            }
            else if (c == ' ') {
                if (!clean_name.empty() && clean_name.back() != '-') {
                    clean_name.push_back('-');
                }
            }
        }

        if (clean_name.empty() || clean_name == "-") {
            return "proxy";
        }

        if (clean_name.back() == '-') {
            clean_name.pop_back();
        }

        return clean_name;
    }

    Config parse_config(std::string_view url)
    {
        Config cfg;

        try {
            // protocol
            size_t pos = url.find("://");
            std::string_view protocol = url.substr(0, pos);
            // Taking suffix of the og string, from pos to end
            url = url.substr(pos + 3);

            // uuid
            pos = url.find('@');
            std::string_view uuid = url.substr(0, pos);
            url = url.substr(pos + 1);

            // address
            pos = url.find_first_of(':');
            std::string_view address = url.substr(0, pos);
            url = url.substr(pos + 1);

            // port
            pos = url.find('?');
            std::string_view port = url.substr(0, pos);
            url = url.substr(pos + 1);

            // remarks
            pos = url.find('#');
            std::string_view remarks;
            if (pos != std::string_view::npos) {
                remarks = url.substr(pos + 1);
                url = url.substr(0, pos);
            }

            // params
            std::map<std::string, std::string> params;
            while (!url.empty()) {
                size_t next_amp = url.find('&');

                std::string_view pair;
                // if its not last pair, slice and continue
                if (next_amp != std::string_view::npos) {
                    pair = url.substr(0, next_amp);
                    url = url.substr(next_amp + 1);
                }
                else {
                    pair = url;
                    url = "";
                }

                size_t eq_pos = pair.find('=');
                if (eq_pos != std::string_view::npos) {
                    std::string_view key = pair.substr(0, eq_pos);
                    std::string val = url_decode(pair.substr(eq_pos + 1));
                    if (key == "fragment") {
                        cfg.fragment = parse_fragment(val);
                    }
                    else if (key == "alpn") {
                        cfg.alpn = parse_alpn(val);
                    }
                    else {
                        params[std::string(key)] = val;
                    }
                }
            }

            cfg.params      = params;
            cfg.protocol    = std::string(protocol);
            cfg.uuid        = std::string(uuid);
            cfg.address     = std::string(address);
            cfg.remarks     = url_decode(remarks);
            cfg.port        = std::stoi(std::string(port));
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to parse config. Invalid url\n";
        }

        return cfg;
    }
}
