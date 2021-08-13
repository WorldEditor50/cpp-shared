#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include <iostream>

template <int N>
class LogWriter
{
public:
    enum Level {
        ALL = 0,
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };
    static std::string logFileName;
    static bool on;
private:
    static std::mutex mutex;
    static const std::vector<std::string> levelString;
public:
    LogWriter() = default;
    static std::string merge(int level, int lineNo, const std::string &fileName, const std::string &function, const std::string &info)
    {
        std::string content;
        auto now = std::chrono::system_clock::now();
        uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
            - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
        time_t tt = std::chrono::system_clock::to_time_t(now);
        auto time_tm = localtime(&tt);
        char strTime[25] = { 0 };
        sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d %03d",
                time_tm->tm_year + 1900,
                time_tm->tm_mon + 1,
                time_tm->tm_mday,
                time_tm->tm_hour,
                time_tm->tm_min,
                time_tm->tm_sec,
                (int)dis_millseconds);
        std::string datetime(strTime);
        content += std::string("[") + datetime + std::string("]");
        content += std::string("[") + levelString[level] + std::string("]");
        content += std::string("[FILE:") + fileName + std::string("]");
        content += std::string("[LINE:") + std::to_string(lineNo) + std::string("]");
        content += std::string("[FUNC:") + function + std::string("]");
        content += std::string("[MESSAGE:") + info + std::string("]");
        return content;
    }
    static void write(int level, int lineNo, const std::string &fileName, const std::string &function, const std::string &info)
    {
        if (on == false) {
            return;
        }
        std::lock_guard<std::mutex> lockguard(mutex);
        std::ofstream file(logFileName, std::ios_base::in | std::ios_base::app);
        if (file.is_open() == false) {
            return;
        }

        std::string content = merge(level, lineNo, fileName, function, info);
        file.write(content.c_str(), content.size());
        file.close();
        return;
    }

    void show(int level, int lineNo, const std::string &fileName, const std::string &function, const std::string &info)
    {
        std::cout<<merge(level, lineNo, fileName, function, info)<<std::endl;
        return;
    }

    static void clear()
    {
        std::lock_guard<std::mutex> lockguard(mutex);
        std::ofstream file(logFileName, std::ios_base::in | std::ios_base::trunc);
        if (file.is_open() == false) {
            return;
        }
        file.close();
    }
};
template<int N>
std::string LogWriter<N>::logFileName = "0.log";
template<int N>
bool LogWriter<N>::on(true);
template<int N>
std::mutex LogWriter<N>::mutex;
template<int N>
const std::vector<std::string> LogWriter<N>::levelString = {"ALL", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
using Log = LogWriter<0>;

#if 1
    #define LOG(level, message) Log::write(level, __LINE__, __FILE__, __FUNCTION__, message)
    #define LOG_SHOW(level, message) Log::show(level, __LINE__, __FILE__, __FUNCTION__, message)
#else
    #define LOG
#endif
#endif // LOGGER_HPP
