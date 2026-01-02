#include "Zyrnix/config.hpp"
#include "Zyrnix/logger.hpp"
#include "Zyrnix/sinks/stdout_sink.hpp"
#include "Zyrnix/sinks/file_sink.hpp"
#include "Zyrnix/sinks/rotating_file_sink.hpp"
#include "Zyrnix/sinks/loki_sink.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
namespace Zyrnix {


std::vector<LoggerConfig> ConfigLoader::configs_;
static std::string g_last_error;

static std::vector<std::string> split_and_trim(const std::string& value) {
    std::vector<std::string> result;
    std::string current;
    for (char c : value) {
        if (c == ',') {
            if (!current.empty()) {
                // trim spaces
                size_t start = current.find_first_not_of(" \t");
                size_t end = current.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos) {
                    result.push_back(current.substr(start, end - start + 1));
                }
            }
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        size_t start = current.find_first_not_of(" \t");
        size_t end = current.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            result.push_back(current.substr(start, end - start + 1));
        }
    }
    return result;
}

bool ConfigLoader::load_from_json(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        g_last_error = "Could not open config file: " + path;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return load_from_json_string(buffer.str());
}

bool ConfigLoader::load_from_json_string(const std::string& json_str) {
    return parse_json_internal(json_str);
}

std::vector<LoggerConfig> ConfigLoader::get_logger_configs() {
    return configs_;
}

std::map<std::string, std::shared_ptr<Logger>> ConfigLoader::create_loggers() {
    std::map<std::string, std::shared_ptr<Logger>> loggers;
    
    for (const auto& config : configs_) {
        std::shared_ptr<Logger> logger;
        
        if (config.async) {
            logger = Logger::create_async(config.name);
        } else {
            logger = std::make_shared<Logger>(config.name);
        }
        
        logger->set_level(config.level);

        // Apply redaction configuration (v1.1.3)
        if (!config.redact_substrings.empty()) {
            logger->set_redact_patterns(split_and_trim(config.redact_substrings));
        }
        if (!config.redact_regexes.empty()) {
            logger->set_redact_regex_patterns(split_and_trim(config.redact_regexes));
        }
        if (!config.redact_presets.empty()) {
            logger->set_redact_pii_presets(split_and_trim(config.redact_presets));
        }
        logger->set_redact_apply_to_cloud_only(config.redact_cloud_only);
        
        for (const auto& sink_type : config.sinks) {
            if (sink_type == "stdout") {
                logger->add_sink(std::make_shared<StdoutSink>());
            } else if (sink_type == "file") {
                auto it = config.sink_params.find("file_path");
                std::string path = (it != config.sink_params.end()) ? it->second : "app.log";
                logger->add_sink(std::make_shared<FileSink>(path));
            } else if (sink_type == "rotating") {
                auto path_it = config.sink_params.find("rotating_path");
                auto size_it = config.sink_params.find("rotating_max_size");
                auto files_it = config.sink_params.find("rotating_max_files");
                
                std::string path = (path_it != config.sink_params.end()) ? path_it->second : "app.log";
                size_t max_size = (size_it != config.sink_params.end()) ? std::stoull(size_it->second) : 10485760;
                size_t max_files = (files_it != config.sink_params.end()) ? std::stoull(files_it->second) : 5;
                
                logger->add_sink(std::make_shared<RotatingFileSink>(path, max_size, max_files));
            } else if (sink_type == "loki") {
#ifndef XLOG_NO_CLOUD_SINKS
                auto url_it = config.sink_params.find("loki_url");
                auto labels_it = config.sink_params.find("loki_labels");
                auto batch_it = config.sink_params.find("loki_batch_size");
                auto flush_it = config.sink_params.find("loki_flush_interval_ms");
                auto timeout_it = config.sink_params.find("loki_timeout_ms");
                auto insecure_it = config.sink_params.find("loki_insecure_skip_verify");
                auto ca_it = config.sink_params.find("loki_ca_cert_path");

                std::string url = (url_it != config.sink_params.end()) ? url_it->second : "";
                std::string labels = (labels_it != config.sink_params.end()) ? labels_it->second : "";

                LokiOptions opts;
                if (batch_it != config.sink_params.end()) {
                    opts.batch_size = static_cast<size_t>(std::stoull(batch_it->second));
                }
                if (flush_it != config.sink_params.end()) {
                    opts.flush_interval_ms = static_cast<uint64_t>(std::stoull(flush_it->second));
                }
                if (timeout_it != config.sink_params.end()) {
                    opts.timeout_ms = static_cast<long>(std::stol(timeout_it->second));
                }
                if (insecure_it != config.sink_params.end()) {
                    std::string v = insecure_it->second;
                    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
                    opts.insecure_skip_verify = (v == "true" || v == "1");
                }
                if (ca_it != config.sink_params.end()) {
                    opts.ca_cert_path = ca_it->second;
                }

                if (!url.empty()) {
                    logger->add_sink(std::make_shared<LokiSink>(url, labels, opts));
                }
#endif
            }
        }
        
        loggers[config.name] = logger;
    }
    
    return loggers;
}

void ConfigLoader::clear() {
    configs_.clear();
}

std::string ConfigLoader::get_last_error() {
    return g_last_error;
}

LogLevel ConfigLoader::parse_log_level(const std::string& level) {
    std::string lower = level;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "trace") return LogLevel::Trace;
    if (lower == "debug") return LogLevel::Debug;
    if (lower == "info") return LogLevel::Info;
    if (lower == "warn" || lower == "warning") return LogLevel::Warn;
    if (lower == "error") return LogLevel::Error;
    if (lower == "critical") return LogLevel::Critical;
    
    return LogLevel::Info; 
}

bool ConfigLoader::parse_json_internal(const std::string& content) {
 
    
    configs_.clear();
    g_last_error.clear();
    
    
    size_t loggers_pos = content.find("\"loggers\"");
    if (loggers_pos == std::string::npos) {
        g_last_error = "Missing \"loggers\" array in configuration";
        return false;
    }

    size_t array_start = content.find('[', loggers_pos);
    if (array_start == std::string::npos) {
        g_last_error = "Malformed configuration: expected '[' after \"loggers\"";
        return false;
    }
    
   
    size_t pos = array_start + 1;
    while (pos < content.length()) {
 
        while (pos < content.length() && (content[pos] == ' ' || content[pos] == '\n' || content[pos] == '\r' || content[pos] == '\t')) {
            pos++;
        }
        
        if (pos >= content.length() || content[pos] == ']') {
            break; 
        }
        
        if (content[pos] != '{') {
            pos++;
            continue;
        }
        
  
        size_t obj_start = pos;
        size_t obj_end = content.find('}', obj_start);
        if (obj_end == std::string::npos) {
            g_last_error = "Malformed logger object in configuration";
            break;
        }
        
        std::string obj = content.substr(obj_start, obj_end - obj_start + 1);
        
        LoggerConfig config;
        

        size_t name_pos = obj.find("\"name\"");
        if (name_pos != std::string::npos) {
            size_t colon = obj.find(':', name_pos);
            size_t quote1 = obj.find('"', colon);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                config.name = obj.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }
        

        size_t level_pos = obj.find("\"level\"");
        if (level_pos != std::string::npos) {
            size_t colon = obj.find(':', level_pos);
            size_t quote1 = obj.find('"', colon);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                std::string level_str = obj.substr(quote1 + 1, quote2 - quote1 - 1);
                config.level = parse_log_level(level_str);
            }
        }
        

        size_t async_pos = obj.find("\"async\"");
        if (async_pos != std::string::npos) {
            size_t colon = obj.find(':', async_pos);
            size_t true_pos = obj.find("true", colon);
            config.async = (true_pos != std::string::npos && true_pos < obj_end);
        }
        

        size_t sinks_pos = obj.find("\"sinks\"");
        if (sinks_pos != std::string::npos) {
            size_t sinks_array_start = obj.find('[', sinks_pos);
            size_t sinks_array_end = obj.find(']', sinks_array_start);
            if (sinks_array_start != std::string::npos && sinks_array_end != std::string::npos) {
                std::string sinks_content = obj.substr(sinks_array_start + 1, sinks_array_end - sinks_array_start - 1);
                

                size_t sink_pos = 0;
                while (sink_pos < sinks_content.length()) {
                    size_t sink_obj_start = sinks_content.find('{', sink_pos);
                    if (sink_obj_start == std::string::npos) break;
                    
                    size_t sink_obj_end = sinks_content.find('}', sink_obj_start);
                    if (sink_obj_end == std::string::npos) break;
                    
                    std::string sink_obj = sinks_content.substr(sink_obj_start, sink_obj_end - sink_obj_start + 1);
                    
            
                    size_t type_pos = sink_obj.find("\"type\"");
                    if (type_pos != std::string::npos) {
                        size_t colon = sink_obj.find(':', type_pos);
                        size_t quote1 = sink_obj.find('"', colon);
                        size_t quote2 = sink_obj.find('"', quote1 + 1);
                        if (quote1 != std::string::npos && quote2 != std::string::npos) {
                            std::string sink_type = sink_obj.substr(quote1 + 1, quote2 - quote1 - 1);
                            config.sinks.push_back(sink_type);
                            
                 
                            if (sink_type == "file" || sink_type == "rotating") {
                                size_t path_pos = sink_obj.find("\"path\"");
                                if (path_pos != std::string::npos) {
                                    size_t colon2 = sink_obj.find(':', path_pos);
                                    size_t pquote1 = sink_obj.find('"', colon2);
                                    size_t pquote2 = sink_obj.find('"', pquote1 + 1);
                                    if (pquote1 != std::string::npos && pquote2 != std::string::npos) {
                                        std::string path = sink_obj.substr(pquote1 + 1, pquote2 - pquote1 - 1);
                                        if (sink_type == "file") {
                                            config.sink_params["file_path"] = path;
                                        } else {
                                            config.sink_params["rotating_path"] = path;
                                        }
                                    }
                                }
                                
                                if (sink_type == "rotating") {
                                    size_t max_size_pos = sink_obj.find("\"max_size\"");
                                    if (max_size_pos != std::string::npos) {
                                        size_t colon3 = sink_obj.find(':', max_size_pos);
                                        size_t num_start = colon3 + 1;
                                        while (num_start < sink_obj.length() && !isdigit(sink_obj[num_start])) num_start++;
                                        size_t num_end = num_start;
                                        while (num_end < sink_obj.length() && isdigit(sink_obj[num_end])) num_end++;
                                        if (num_start < num_end) {
                                            config.sink_params["rotating_max_size"] = sink_obj.substr(num_start, num_end - num_start);
                                        }
                                    }
                                    
                                    size_t max_files_pos = sink_obj.find("\"max_files\"");
                                    if (max_files_pos != std::string::npos) {
                                        size_t colon4 = sink_obj.find(':', max_files_pos);
                                        size_t num_start = colon4 + 1;
                                        while (num_start < sink_obj.length() && !isdigit(sink_obj[num_start])) num_start++;
                                        size_t num_end = num_start;
                                        while (num_end < sink_obj.length() && isdigit(sink_obj[num_end])) num_end++;
                                        if (num_start < num_end) {
                                            config.sink_params["rotating_max_files"] = sink_obj.substr(num_start, num_end - num_start);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    sink_pos = sink_obj_end + 1;
                }
            }
        }

        // Optional redaction configuration (v1.1.3)
        size_t rs_pos = obj.find("\"redact_substrings\"");
        if (rs_pos != std::string::npos) {
            size_t colon = obj.find(':', rs_pos);
            size_t quote1 = obj.find('"', colon);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                config.redact_substrings = obj.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        size_t rr_pos = obj.find("\"redact_regexes\"");
        if (rr_pos != std::string::npos) {
            size_t colon = obj.find(':', rr_pos);
            size_t quote1 = obj.find('"', colon);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                config.redact_regexes = obj.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        size_t rp_pos = obj.find("\"redact_presets\"");
        if (rp_pos != std::string::npos) {
            size_t colon = obj.find(':', rp_pos);
            size_t quote1 = obj.find('"', colon);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                config.redact_presets = obj.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        size_t rco_pos = obj.find("\"redact_cloud_only\"");
        if (rco_pos != std::string::npos) {
            size_t colon = obj.find(':', rco_pos);
            size_t true_pos = obj.find("true", colon);
            config.redact_cloud_only = (true_pos != std::string::npos && true_pos < obj_end);
        }
        
        if (!config.name.empty()) {
            configs_.push_back(config);
        }
        
        pos = obj_end + 1;
    }
    
    if (configs_.empty()) {
        if (g_last_error.empty()) {
            g_last_error = "No valid logger configurations found";
        }
        return false;
    }

    return true;
}

Config::Config() {}

}
