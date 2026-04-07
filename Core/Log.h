#pragma once

#include <string>

#define MAKE_LEVEL_TYPE \
    X(NORMAL) \
    X(INFO) \
    X(WARNING) \
    X(ERROR)

enum class LogTarget : unsigned short
{
    NONE = 0,
    CONSOLE = 1 << 0,
    FILE = 1 << 1,
    ALL = CONSOLE | FILE
};

#define X(type) type,
enum class LogLevel { MAKE_LEVEL_TYPE COUNT };
#undef X

class Log
{
public:
    static Log& GetInstance();
    void Output(LogLevel level, const char* message, LogTarget target = LogTarget::ALL); // 로그 출력 메소드

private:
    Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static constexpr const char* GetLevelString(LogLevel level);
    static std::string GetCurrentTimeString();

    static void SetConsoleColor(LogLevel level);     // Log Level에 따라서 색상 설정 메소드
    static void ResetConsoleColor();                 // 콘솔 색상 리셋 메소드
    static const char* GetAnsiColorPrefix(LogLevel level); // Log Level에 따라서 ANSI 색상 코드 접두사 반환 (로그 파일에 삽입용)

    void LogMessage(LogLevel level, const char* message);
    void LogConsole(LogLevel level, const char* message);

    std::string LogWrite(LogLevel level, const char* message); // 로그 메시지 기록 메소드
};

// LogTarget 비트마스크 연산자 오버로드
inline LogTarget operator|(LogTarget a, LogTarget b)
{
    return static_cast<LogTarget>(static_cast<unsigned short>(a) | static_cast<unsigned short>(b));
}

inline LogTarget& operator|=(LogTarget& a, LogTarget b)
{
    a = a | b;
    return a;
}

inline bool operator&(LogTarget a, LogTarget b)
{
    return static_cast<LogTarget>(static_cast<unsigned short>(a) & static_cast<unsigned short>(b)) != static_cast<LogTarget>(0);
}

// 로그 레벨을 문자열로 변환
inline constexpr const char* Log::GetLevelString(LogLevel level)
{
#define X(type) case LogLevel::type: return #type;
    switch (level)
    {
        MAKE_LEVEL_TYPE

    default:
        return "UNKNOWN";
    }
#undef X
}
