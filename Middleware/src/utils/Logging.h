#pragma once
#ifndef MIDDLEWARE__LOGGING__H
    #define MIDDLEWARE__LOGGING__H

    #include <string>
    #include <sstream>
    #include <iostream>
    #include <stdarg.h>
    #include <chrono>
    #include <format>
    #include <mutex>

    namespace Middleware::Utils {

        enum LogLevel {
            LOG_LEVEL_NONE = 0,
            LOG_LEVEL_TRACE = 1,
            LOG_LEVEL_INFO = 2,
            LOG_LEVEL_WARN = 3,
            LOG_LEVEL_PERFORMANCE = 4,
            LOG_LEVEL_ERROR = 5,
            LOG_LEVEL_FATAL = 6,
            LOG_LEVEL_TEST = 7
        };

        static const char* LOG_LEVEL_PREFIXES[] = {
            "[NONE]    ",
            "[TRACE]   ",
            "[INFO]    ",
            "[WARN]    ",
            "[PERFORM] ",
            "[ERROR]   ",
            "[FATAL]   "
            "[TEST]    "
        };

        static const char* LOG_LEVEL_ANSI_PREFIXES[] = {
            "\33[90m", // Light-grey text
            "\33[90m", // Light-grey text
            "\33[36m", // Cyan text
            "\33[93m", // Yellow text 
            "\33[35m", // Purple text 
            "\33[91m", // Red text
            "\33[101m" // Red highlight, white text
            "\33[90m", // Light-grey text
        };

        class Logger {
            public:
                Logger(const std::string logName = "", const std::string logNameSuffix = "", LogLevel logLevel = LOG_LEVEL_NONE) : m_LogName(logName), m_LogNameSuffix(logNameSuffix), m_CurrentLogLevel(logLevel) { }

                inline void Trace(const std::string& filepath, const std::string& methodName, size_t lineNumber = 0, const std::string message = "", ...) {
                    if(!ShouldAcceptLogMessage(LOG_LEVEL_TRACE)) return;
                    va_list args;
                    va_start(args, message);
                    Log(LOG_LEVEL_TRACE, filepath, methodName, lineNumber, message, args);
                    va_end(args);
                }

                inline void Info(const std::string& filepath, const std::string& methodName, size_t lineNumber = 0, const std::string message = "", ...) {
                    if(!ShouldAcceptLogMessage(LOG_LEVEL_INFO)) return;
                    va_list args;
                    va_start(args, message);
                    Log(LOG_LEVEL_INFO, filepath, methodName, lineNumber, message, args);
                    va_end(args);
                }

                inline void Warn(const std::string& filepath, const std::string& methodName, size_t lineNumber = 0, const std::string message = "", ...) {
                    if(!ShouldAcceptLogMessage(LOG_LEVEL_WARN)) return;
                    va_list args;
                    va_start(args, message);
                    Log(LOG_LEVEL_WARN, filepath, methodName, lineNumber, message, args);
                    va_end(args);
                }

                inline void Performance(const std::string& filepath, const std::string& methodName, size_t lineNumber = 0, const std::string message = "", ...) {
                    if(!ShouldAcceptLogMessage(LOG_LEVEL_PERFORMANCE)) return;
                    va_list args;
                    va_start(args, message);
                    Log(LOG_LEVEL_PERFORMANCE, filepath, methodName, lineNumber, message, args);
                    va_end(args);
                }

                inline void Error(const std::string& filepath, const std::string& methodName, size_t lineNumber = 0, const std::string message = "", ...) {
                    if(!ShouldAcceptLogMessage(LOG_LEVEL_ERROR)) return;
                    va_list args;
                    va_start(args, message);
                    Log(LOG_LEVEL_ERROR, filepath, methodName, lineNumber, message, args);
                    va_end(args);
                }

                inline void Fatal(const std::string& filepath, const std::string& methodName, size_t lineNumber = 0, const std::string message = "", ...) {
                    if(!ShouldAcceptLogMessage(LOG_LEVEL_FATAL)) return;
                    va_list args;
                    va_start(args, message);
                    Log(LOG_LEVEL_FATAL, filepath, methodName, lineNumber, message, args);
                    va_end(args);
                }

                inline void Test(const std::string& filepath, const std::string& methodName, size_t lineNumber = 0, const std::string message = "", ...) {
                    if(!ShouldAcceptLogMessage(LOG_LEVEL_TEST)) return;
                    va_list args;
                    va_start(args, message);
                    Log(LOG_LEVEL_TEST, filepath, methodName, lineNumber, message, args);
                    va_end(args);
                }

            private:
                std::string m_LogName;
                std::string m_LogNameSuffix;
                LogLevel m_CurrentLogLevel = LOG_LEVEL_NONE;

            private:
                void GetShortenedFilepath(const std::string& filepath, std::string& output) const {
                    int startOfShortenedFilepath = ((int) filepath.length()) - 1;
                    for(int i = ((int) (filepath.length() - 1)); i >= 0; i--) {
                        char currentFilepathChar = filepath.at(i);
                        if((currentFilepathChar == '/') || (currentFilepathChar == '\\')) {
                            startOfShortenedFilepath = i + 1;
                            break;
                        }
                    }
                    output = filepath.substr(startOfShortenedFilepath);
                }

                void ProduceMessage(const std::string& message, va_list args, std::string& output) const {
                    size_t messageIndex = 0;
                    while(messageIndex < message.length()) {
                        const char currentChar = message.at(messageIndex);

                        switch(currentChar) {
                            case '%':
                                {
                                    if((messageIndex + 1) < message.length()) {
                                        const char nextChar = message.at(messageIndex + 1);
                                        switch(nextChar) {
                                            case 'd': case 'i':
                                                {
                                                    int integer = va_arg(args, int);
                                                    std::string integerString;
                                                    std::stringstream stream;
                                                    stream << integer;
                                                    stream >> integerString;
                                                    output.append(integerString);
                                                }
                                                break;

                                            case 'f':
                                            {
                                                float floatValue = va_arg(args, float);
                                                std::string floatString;
                                                std::stringstream stream;
                                                stream << floatValue;
                                                stream >> floatString;
                                                output.append(floatString);
                                            }
                                            break;

                                            case 's':
                                                {
                                                    const char* string = va_arg(args, const char*);
                                                    output.append(string);
                                                }
                                                break;

                                            case 'b':
                                                {
                                                    const int value = va_arg(args, int);
                                                    if(value == 0) {
                                                        output.append("false");
                                                    } else {
                                                        output.append("true");
                                                    }
                                                }
                                                break;

                                            case '%':
                                                output.append(1, '%');
                                                break;

                                            default:
                                                // If we don't recognise the code then add the code as-is to the logs
                                                output.append(message, messageIndex, 2);
                                                break;
                                        }
                                        messageIndex += 2;
                                    } else {
                                        output.append(1, message.at(messageIndex));
                                        messageIndex++;
                                    }
                                }
                                break;

                            default:
                                output.append(1, message.at(messageIndex));
                                messageIndex++;
                                break;
                        }

                    }
                }

                void Log(LogLevel level, const std::string& filepath, const std::string& methodName, size_t lineNumber, const std::string& message, va_list args) {
                    const std::string consolePrefix = ((level >= LOG_LEVEL_NONE) && (level <= LOG_LEVEL_FATAL) ? std::string(LOG_LEVEL_PREFIXES[(int) level]) : "");
                    const std::string consoleAnsiPrefix = ((level >= LOG_LEVEL_NONE) && (level <= LOG_LEVEL_FATAL) ? std::string(LOG_LEVEL_ANSI_PREFIXES[(int) level]) : "");

                    // Get the source file that requested the log message
                    std::string consoleFriendlyFilepath;
                    GetShortenedFilepath(filepath, consoleFriendlyFilepath);
                    int charsBelowFilepathLengthAverage = (20 - ((int) consoleFriendlyFilepath.length()));
                    std::string consoleFriendlyFilepathSuffix;
                    if(charsBelowFilepathLengthAverage > 0) consoleFriendlyFilepathSuffix.append(charsBelowFilepathLengthAverage, ' ');

                    // Get the method that requested the log message
                    int charsBelowMethodNameLengthAverage = (40 - ((int) methodName.length()));
                    std::string methodNameSuffx;
                    if(charsBelowMethodNameLengthAverage > 0) methodNameSuffx.append(charsBelowMethodNameLengthAverage, ' ');

                    // Get the line number within the source file that requested the log message
                    std::string lineNumberString;
                    std::stringstream lineNumberStringStream;
                    lineNumberStringStream << lineNumber;
                    lineNumberStringStream >> lineNumberString;
                    int charsBelowLineNumberLengthAverage = (4 - ((int) lineNumberString.length()));
                    std::string lineNumberStringPrefix;
                    if(charsBelowLineNumberLengthAverage > 0) lineNumberStringPrefix.append(charsBelowLineNumberLengthAverage, ' ');

                    // Assemble the message provided and additional parameters into a final log message
                    std::string producedMessage;
                    ProduceMessage(message, args, producedMessage);

                    // Retrieve the current date/time that this log message is being sent to the console
                    const auto currentTime = std::chrono::system_clock::now();
                    const std::string currentTimeString = std::format("{:%d-%m-%Y %H:%M:%OS}", currentTime);

                    static std::mutex consoleOutputMutex;

                    consoleOutputMutex.lock();
                    std::cout << consoleAnsiPrefix << "{" << m_LogName << "} " << m_LogNameSuffix;
                    std::cout << consolePrefix;
                    std::cout << consoleFriendlyFilepath << consoleFriendlyFilepathSuffix << " | ";
                    std::cout << methodName << methodNameSuffx << " | ";
                    std::cout << lineNumberStringPrefix << lineNumberString << " | ";
                    std::cout << currentTimeString << " | ";
                    std::cout << producedMessage << "\33[m" << std::endl;
                    consoleOutputMutex.unlock();
                }

                inline bool ShouldAcceptLogMessage(LogLevel level) const { return (level >= m_CurrentLogLevel); }
        };

        static Logger MIDDLEWARE_LOGGER_INSTANCE("Middleware", "", LOG_LEVEL_NONE);
        static Logger SERVER_LOGGER_INSTANCE("Server", "    ", LOG_LEVEL_NONE);
        static Logger CLIENT_LOGGER_INSTANCE("Client", "    ", LOG_LEVEL_NONE);

    }

    #ifdef DGS_BUILD_DEBUG

        #define MIDDLEWARE_TRACE(msg, ...) Middleware::Utils::MIDDLEWARE_LOGGER_INSTANCE.Trace(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        #define MIDDLEWARE_INFO(msg, ...) Middleware::Utils::MIDDLEWARE_LOGGER_INSTANCE.Info(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        #define MIDDLEWARE_WARN(msg, ...) Middleware::Utils::MIDDLEWARE_LOGGER_INSTANCE.Warn(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        
        #define SERVER_TRACE(msg, ...) Middleware::Utils::SERVER_LOGGER_INSTANCE.Trace(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        #define SERVER_INFO(msg, ...) Middleware::Utils::SERVER_LOGGER_INSTANCE.Info(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        #define SERVER_WARN(msg, ...) Middleware::Utils::SERVER_LOGGER_INSTANCE.Warn(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        
        #define CLIENT_TRACE(msg, ...) Middleware::Utils::CLIENT_LOGGER_INSTANCE.Trace(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        #define CLIENT_INFO(msg, ...) Middleware::Utils::CLIENT_LOGGER_INSTANCE.Info(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
        #define CLIENT_WARN(msg, ...) Middleware::Utils::CLIENT_LOGGER_INSTANCE.Warn(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)

    #else

        #define MIDDLEWARE_TRACE(msg, ...) 0
        #define MIDDLEWARE_INFO(msg, ...) 0
        #define MIDDLEWARE_WARN(msg, ...) 0
        
        #define SERVER_TRACE(msg, ...) 0
        #define SERVER_INFO(msg, ...) 0
        #define SERVER_WARN(msg, ...) 0
        
        #define CLIENT_TRACE(msg, ...) 0
        #define CLIENT_INFO(msg, ...) 0
        #define CLIENT_WARN(msg, ...) 0

    #endif

    #define MIDDLEWARE_PERFORM(msg, ...) Middleware::Utils::MIDDLEWARE_LOGGER_INSTANCE.Performance(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define MIDDLEWARE_ERROR(msg, ...) Middleware::Utils::MIDDLEWARE_LOGGER_INSTANCE.Error(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define MIDDLEWARE_FATAL(msg, ...) Middleware::Utils::MIDDLEWARE_LOGGER_INSTANCE.Fatal(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define MIDDLEWARE_TEST(msg, ...) Middleware::Utils::MIDDLEWARE_LOGGER_INSTANCE.Test(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)

    #define SERVER_PERFORM(msg, ...) Middleware::Utils::SERVER_LOGGER_INSTANCE.Performance(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define SERVER_ERROR(msg, ...) Middleware::Utils::SERVER_LOGGER_INSTANCE.Error(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define SERVER_FATAL(msg, ...) Middleware::Utils::SERVER_LOGGER_INSTANCE.Fatal(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define SERVER_TEST(msg, ...) Middleware::Utils::SERVER_LOGGER_INSTANCE.Test(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)

    #define CLIENT_PERFORM(msg, ...) Middleware::Utils::CLIENT_LOGGER_INSTANCE.Performance(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define CLIENT_ERROR(msg, ...) Middleware::Utils::CLIENT_LOGGER_INSTANCE.Error(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define CLIENT_FATAL(msg, ...) Middleware::Utils::CLIENT_LOGGER_INSTANCE.Fatal(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)
    #define CLIENT_TEST(msg, ...) Middleware::Utils::CLIENT_LOGGER_INSTANCE.Test(__FILE__, __func__, __LINE__, msg __VA_OPT__(,) __VA_ARGS__)

#endif