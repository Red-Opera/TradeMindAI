#pragma once

class Config
{
public:
    // 루트 디렉터리 경로 설정
#ifdef DEBUG
    static constexpr const char* rootPath = "~/TradeMindAI/Run";
#else
    static constexpr const char* rootPath = "~/TradeMindAI/Test";
#endif
};
