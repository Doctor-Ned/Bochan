#include "pch.h"
#include "BochanLog.h"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> bochan::BochanLog::logger{};
const std::string bochan::BochanLog::BOCHAN_LOG_NAME{ "Bochan" };
const std::string bochan::BochanLog::BOCHAN_LOG_FILENAME{ "bochan.log" };
const size_t bochan::BochanLog::BOCHAN_LOG_FILE_SIZE{ 8'388'608UL };
const size_t bochan::BochanLog::BOCHAN_LOG_MAX_FILES{ 3UL };

spdlog::logger* bochan::BochanLog::getLogger() {
    if (!logger) {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(BOCHAN_LOG_FILENAME, BOCHAN_LOG_FILE_SIZE, BOCHAN_LOG_MAX_FILES);
        spdlog::sinks_init_list sinkList{ consoleSink, rotatingFileSink };
        logger = std::make_shared<spdlog::logger>(BOCHAN_LOG_NAME, sinkList);
        spdlog::set_default_logger(logger);
    }
    return logger.get();
}
