#pragma once

#include "spdlog/spdlog.h"

#define BOCHAN_LEVEL_TRACE 0
#define BOCHAN_LEVEL_DEBUG 1
#define BOCHAN_LEVEL_INFO 2
#define BOCHAN_LEVEL_WARN 3
#define BOCHAN_LEVEL_ERROR 4
#define BOCHAN_LEVEL_CRITICAL 5
#define BOCHAN_LEVEL_OFF 6

#ifndef BOCHAN_LOG_LEVEL
#define BOCHAN_LOG_LEVEL BOCHAN_LEVEL_DEBUG
#endif

#define BOCHAN_LOGGER_CALL(logger, level, ...) (logger)->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)

#if BOCHAN_LOG_LEVEL <= BOCHAN_LEVEL_TRACE
#define BOCHAN_LOGGER_TRACE(logger, ...) BOCHAN_LOGGER_CALL(bochan::BochanLog::getLogger(), spdlog::level::trace, __VA_ARGS__)
#define BOCHAN_TRACE(...) BOCHAN_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define BOCHAN_LOGGER_TRACE(logger, ...) (void)0
#define BOCHAN_TRACE(...) (void)0
#endif

#if BOCHAN_LOG_LEVEL <= BOCHAN_LEVEL_DEBUG
#define BOCHAN_LOGGER_DEBUG(logger, ...) BOCHAN_LOGGER_CALL(bochan::BochanLog::getLogger(), spdlog::level::debug, __VA_ARGS__)
#define BOCHAN_DEBUG(...) BOCHAN_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define BOCHAN_LOGGER_DEBUG(logger, ...) (void)0
#define BOCHAN_DEBUG(...) (void)0
#endif

#if BOCHAN_LOG_LEVEL <= BOCHAN_LEVEL_INFO
#define BOCHAN_LOGGER_INFO(logger, ...) BOCHAN_LOGGER_CALL(bochan::BochanLog::getLogger(), spdlog::level::info, __VA_ARGS__)
#define BOCHAN_INFO(...) BOCHAN_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define BOCHAN_LOGGER_INFO(logger, ...) (void)0
#define BOCHAN_INFO(...) (void)0
#endif

#if BOCHAN_LOG_LEVEL <= BOCHAN_LEVEL_WARN
#define BOCHAN_LOGGER_WARN(logger, ...) BOCHAN_LOGGER_CALL(bochan::BochanLog::getLogger(), spdlog::level::warn, __VA_ARGS__)
#define BOCHAN_WARN(...) BOCHAN_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define BOCHAN_LOGGER_WARN(logger, ...) (void)0
#define BOCHAN_WARN(...) (void)0
#endif

#if BOCHAN_LOG_LEVEL <= BOCHAN_LEVEL_ERROR
#define BOCHAN_LOGGER_ERROR(logger, ...) BOCHAN_LOGGER_CALL(bochan::BochanLog::getLogger(), spdlog::level::err, __VA_ARGS__)
#define BOCHAN_ERROR(...) BOCHAN_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define BOCHAN_LOGGER_ERROR(logger, ...) (void)0
#define BOCHAN_ERROR(...) (void)0
#endif

#if BOCHAN_LOG_LEVEL <= BOCHAN_LEVEL_CRITICAL
#define BOCHAN_LOGGER_CRITICAL(logger, ...) BOCHAN_LOGGER_CALL(bochan::BochanLog::getLogger(), spdlog::level::critical, __VA_ARGS__)
#define BOCHAN_CRITICAL(...) BOCHAN_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define BOCHAN_LOGGER_CRITICAL(logger, ...) (void)0
#define BOCHAN_CRITICAL(...) (void)0
#endif

namespace bochan {
    class BochanLog sealed {
    public:
        BOCHANAPI BochanLog() = delete;
        static BOCHANAPI spdlog::logger* getLogger();
    private:
        static BOCHANAPI std::shared_ptr<spdlog::logger> logger;
        static BOCHANAPI const std::string BOCHAN_LOG_NAME;
        static BOCHANAPI const std::string BOCHAN_LOG_FILENAME;
        static BOCHANAPI const size_t BOCHAN_LOG_FILE_SIZE;
        static BOCHANAPI const size_t BOCHAN_LOG_MAX_FILES;
    };
}