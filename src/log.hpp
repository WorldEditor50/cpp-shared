#ifndef LOG_HPP
#define LOG_HPP
#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include <iostream>

class Log
{
public:
    bool on;
    std::string logFileName;
    std::mutex mutex;
public:
    Log(){}
    explicit Log(const std::string &fileName):on(true),logFileName(fileName){}
    void write(const std::string &level, int lineNo, const std::string &fileName, const std::string &function, const std::string &info)
    {
        if (on == false) {
            return;
        }
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
        content += std::string("[") + level + std::string("]");
        content += std::string("[FILE:") + fileName + std::string("]");
        content += std::string("[LINE:") + std::to_string(lineNo) + std::string("]");
        content += std::string("[FUNC:") + function + std::string("]");
        content += std::string("[MESSAGE:") + info + std::string("]\n");
        /* write */
        std::lock_guard<std::mutex> lockguard(mutex);
        std::ofstream file(logFileName, std::ios_base::in | std::ios_base::app);
        if (file.is_open() == false) {
            return;
        }
        file.write(content.c_str(), content.size());
        file.close();
        return;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lockguard(mutex);
        std::ofstream file(logFileName, std::ios_base::in | std::ios_base::trunc);
        if (file.is_open() == false) {
            return;
        }
        file.close();
        return;
    }
};

Log xlog("xlog.log");
#if 1
    #define LOG_INFO(message)  xlog.write("INFO", __LINE__, __FILE__, __FUNCTION__, message)
    #define LOG_DEBUG(message) xlog.write("DEBUG", __LINE__, __FILE__, __FUNCTION__, message)
    #define LOG_ERROR(message) xlog.write("ERROR", __LINE__, __FILE__, __FUNCTION__, message)
    #define LOG_FATAL(message) xlog.write("FATAL", __LINE__, __FILE__, __FUNCTION__, message)
#else
    #define LOG
#endif

#endif // LOG_HPP
