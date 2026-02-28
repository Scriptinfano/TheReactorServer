#include "log.hpp"
#include <cstring>
#include <cerrno>
#include <ctime>
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <string>

// 类中的私有静态成员变量必须在cpp文件中先声明一遍，然后才能在静态成员函数中访问
// 在 C++ 中，静态成员变量属于类本身而不是任何具体的对象。由于它们在类中只是声明而没有定义，因此需要在类外面对它们进行定义，这就相当于为静态成员变量分配存储空间并指定其初始值。
const char *Logger::num2strLevelMap[] = {"DEBUG", "NORMAL", "WARNING", "ERROR", "FATAL", "OFF"};
const std::map<std::string, LogLevel> Logger::str2numLevelMap = {
    {"DEBUG", LogLevel::DEBUG},
    {"NORMAL", LogLevel::NORMAL},
    {"WARNING", LogLevel::WARNING},
    {"ERROR", LogLevel::ERROR},
    {"FATAL", LogLevel::FATAL},
    {"OFF", LogLevel::OFF}};
const char *Logger::pname_ = nullptr;
////////////////////////////////////////////////////////////////////

/*
只属于这个cpp文件的静态函数，不在log.hpp文件中声明，因为这个函数只在这个文件中使用
*/
static std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;
    while (std::getline(stream, token, delimiter))
    {
        // 去掉前后空格
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        tokens.push_back(token);
    }
    return tokens;
}

Logger::Logger()
{
    std::ifstream configFile("./etc/config.txt");
    if (!configFile.is_open())
    {
        std::cerr << "failed to open configuration file!" << std::endl;
        exit(-1);
    }
    std::string line;
    while (std::getline(configFile, line))
    {
        // 去除每一行的前后空格和制表符
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        // 忽略空行和注释
        if (line.empty() || line[0] == '#')
            continue;
        if (line.rfind("SHOULDLOGTOFILE", 0) == 0)
        {
            auto pos = line.find('=');
            if (pos != std::string::npos)
            {
                std::string value = line.substr(pos + 1);
                logToFileLevels_ = split(value, ',');
            }
            std::cout << "SHOUDLDLOGTOLEVEL 配置项加载完成，其值为";
            for (auto &str : logToFileLevels_)
                std::cout << str << ",";
            std::cout << std::endl;
        }
        if (line.rfind("LOGLEVEL", 0) == 0)
    {
        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            stringLogLevel_ = line.substr(pos + 1);
            // 去掉前后空格
            stringLogLevel_.erase(0, stringLogLevel_.find_first_not_of(" \t"));
            stringLogLevel_.erase(stringLogLevel_.find_last_not_of(" \t") + 1);
        }
        
        auto it = str2numLevelMap.find(stringLogLevel_);
        if (it != str2numLevelMap.end()) {
             numLogLevel_ = it->second;
        } else {
             // 只有当 stringLogLevel_ 不为空且不是有效级别时才报错
             if (!stringLogLevel_.empty()) {
                 std::cerr << "Invalid LOGLEVEL: " << stringLogLevel_ << ", defaulting to DEBUG" << std::endl;
             }
             numLogLevel_ = DEBUG;
        }

        std::cout << "LOGLEVEL 配置项加载完成，其值为";
        std::cout << "字符串形式=" << stringLogLevel_ << "," << "数字形式=" << numLogLevel_ << std::endl;
    }
    }
}

Logger::~Logger() {}

void Logger::setLoggerPname(const char *pname)
{
    Logger::pname_ = pname;
}

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

std::string Logger::createErrorMessage(const std::string &msg)
{
    const char *error_str = strerror(errno);
    return msg + ": " + error_str;
}

bool Logger::shouldLogToFile(LogLevel level)
{
    std::string theLevel = num2strLevelMap[level];
    for (auto &str : logToFileLevels_)
    {
        if (theLevel == str)
        {
            return true;
        }
    }
    return false;
}

void Logger::logMessage(LogLevel level, const char *file, int line, const char *format, ...)
{

    if (level < DEBUG || level > OFF)
    {
        this->logMessage(FATAL, __FILE__, __LINE__, "invalid log level:%s", num2strLevelMap[level]);
        exit(-1);
    }

    // 仅输出规定级别以上的日志
    if (level < numLogLevel_)
        return;
    if (numLogLevel_ == OFF) return;

    char fixBuffer[512];
    std::time_t currentTime = std::time(nullptr);
    char timestr[64];
    if (std::strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime)) == 0)
    {
        this->logMessage(FATAL, __FILE__, __LINE__, "failed to format time");
        exit(-1);
    }

    snprintf(fixBuffer, sizeof(fixBuffer), "<%s>==[%s:%s:%d][%s]",
             num2strLevelMap[level], pname_, file, line, timestr);

    char defBuffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(defBuffer, sizeof(defBuffer), format, args);
    va_end(args);

    if (shouldLogToFile(level))
    {
        std::ofstream logFile(LOGFILE, std::ios_base::app);
        if (!logFile.is_open())
        {
            this->logMessage(FATAL, __FILE__, __LINE__, "failed to open log file %s", LOGFILE.c_str());
            exit(-1);
        }
        logFile << fixBuffer << ":" << defBuffer << std::endl;
    }
    std::cout << fixBuffer << ":" << defBuffer << std::endl;
}
Logger &logger = Logger::getInstance(); // 初始化全局 Logger 实例引用，这样其他所有模块只要包含了log.hpp，就可以引用这个全局唯一的logger输出标准化的日志