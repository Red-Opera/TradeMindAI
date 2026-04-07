#include "Log.h"
#include "Config.h"

#include "../Utility/Directory.h"

#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

static std::mutex g_logMutex;

void Log::SetConsoleColor(LogLevel level)
{
    switch (level)
    {
    case LogLevel::NORMAL:
        std::cout << "\033[0m"; // reset
        break;

    case LogLevel::INFO:
        std::cout << "\033[32m"; // green
        break;

    case LogLevel::WARNING:
        std::cout << "\033[33m"; // yellow
        break;

    case LogLevel::ERROR:
        // Error는 굵은 빨간색으로 표시
        std::cout << "\033[1;31m"; // bold red
        break;

    default:
        std::cout << "\033[0m";
        break;
    }
}

void Log::ResetConsoleColor()
{
    std::cout << "\033[0m";
}

const char* Log::GetAnsiColorPrefix(LogLevel level)
{
    switch (level)
    {
    case LogLevel::NORMAL:  return "\033[0m";    // reset / default
    case LogLevel::INFO:    return "\033[32m";   // green
    case LogLevel::WARNING: return "\033[33m";   // yellow
    case LogLevel::ERROR:   return "\033[1;31m"; // bold red
    default:                return "\033[0m";
    }
}

Log& Log::GetInstance()
{
    static Log instance;

    return instance;
}

// 로그 출력 메소드
void Log::Output(LogLevel level, const char* message, LogTarget target)
{
    if (target & LogTarget::CONSOLE)
        LogConsole(level, message);

    if (target & LogTarget::FILE)
        LogMessage(level, message);
}

// 현재 시간 문자열 반환 함수
std::string Log::GetCurrentTimeString()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);

    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);

    return std::string(timeBuf);
}

// 로그 메시지 포맷팅 ([시간] [레벨] 메시지)
std::string Log::LogWrite(LogLevel level, const char* message)
{
    std::ostringstream oss;
    oss << "[" << GetCurrentTimeString() << "] [" << GetLevelString(level) << "] " << message;

    return oss.str();
}

void Log::LogConsole(LogLevel level, const char* message)
{
    // 스레드 안전하게 콘솔에 로그 출력
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::string line = LogWrite(level, message);

    // LogLevel에 따라 콘솔 색상 설정 후 출력, 리셋
    SetConsoleColor(level);

    std::cout << line << std::endl;

    ResetConsoleColor();
}

void Log::LogMessage(LogLevel level, const char* message)
{
    std::lock_guard<std::mutex> lock(g_logMutex);

    // 포맷된 로그 메시지 생성
    std::string line = LogWrite(level, message);

    // log 파일 경로 결정 : Config::rootPath의 절대 경로 + "/log.txt"
    std::string dir = Directory::GetAbsolutePath(std::string(Config::rootPath));

    if (!dir.empty() && dir.back() != '/')
        dir.push_back('/');

    std::string logPath = dir + "log.txt";

    // 로그 메시지를 파일에 기록
    std::ofstream logFile;
    logFile.open(logPath, std::ios::app);

    if (!logFile.is_open())
    {
        // 폴백: 현재 작업 디렉터리에 log.txt를 생성
        const char* localPath = "log.txt";
        logFile.open(localPath, std::ios::app);
    }

    if (logFile.is_open())
    {
        // 탑재된 ANSI 색상 코드를 사용하여 로그 파일에 기록
        const char* colorPrefix = GetAnsiColorPrefix(level);
        const char* colorSuffix = "\033[0m";

        logFile << colorPrefix << line << colorSuffix << std::endl;
        logFile.close();
    }
}

Log::Log()
{
    // 로그 파일 경로 결정 : Config::rootPath의 절대 경로 + "/log.txt"
    std::string dir = Directory::GetAbsolutePath(std::string(Config::rootPath));

    // 디렉터리 생성 시도
    bool isSucceedDir = Directory::EnsureDirectoryExists(dir);

    assert(isSucceedDir && "Failed to create logging directory");

    if (!dir.empty() && dir.back() != '/')
        dir.push_back('/');

    std::string logPath = dir + "log.txt";

    // 로그 파일 존재 및 내용 확인
    bool hasLogFileAndNotEmpty = false;
    std::ifstream inputFileStream(logPath, std::ios::binary | std::ios::ate);

    if (inputFileStream.is_open())
    {
        // 파일 크기 확인
        std::streamsize size = inputFileStream.tellg();

        if (size > 0)
            hasLogFileAndNotEmpty = true;

        inputFileStream.close();
    }

    // 생성 또는 헤더 추가
    std::ofstream outFileStream(logPath, std::ios::app);

    if (outFileStream)
    {
        std::string timeStr = GetCurrentTimeString();

        if (hasLogFileAndNotEmpty)
            outFileStream << "\n---- New session started (existing log has data) [" << timeStr << "] ----\n\n";

        else
            outFileStream << "\n---- New session started (empty log file) [" << timeStr << "] ----\n\n";
    }

    else
    {
        // 루트에 생성 실패(권한 등). 현재 작업 디렉터리에 폴백하여 생성
        const char* localPath = "log.txt";
        std::ofstream outStream(localPath, std::ios::app);

        if (outStream)
        {
            std::string timeStr = GetCurrentTimeString();
            outStream << "\n---- New session started (fallback) [" << timeStr << "] ----\n\n";
        }
        // 실패해도 무시하고 계속 진행
    }
}
