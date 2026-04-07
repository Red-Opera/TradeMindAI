#pragma once

#include <string>
#include <string_view>

class Directory
{
public:
    static bool HasDirExists(const std::string_view& path);                 // 디렉토리가 존재하는지 확인
    static bool MakeDir(const std::string_view& path);                      // 디렉토리 생성
    static bool EnsureDirectoryExists(const std::string_view& path);        // 디렉토리가 존재하지 않으면 생성

    static std::string GetAbsolutePath(const std::string_view& path);       // 절대 경로 가져오기
    static std::string ConvertHomeToFull(const std::string_view& inPath);   // 경로에 홈 디렉토리(~)가 포함되어 있으면 확장
};
