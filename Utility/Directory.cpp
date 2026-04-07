#include "Directory.h"

#include <cerrno>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool Directory::HasDirExists(const std::string_view& path)
{
    struct stat info;

    // 경로의 상태 정보를 가져옴
    if (stat(path.data(), &info) != 0)
        return false;

    // 디렉토리인지 확인
    return S_ISDIR(info.st_mode);
}

bool Directory::MakeDir(const std::string_view& path)
{
    if (mkdir(path.data(), 0755) == 0)
        return true;

    // 디렉토리가 이미 존재하는 경우
    return errno == EEXIST;
}

std::string Directory::GetAbsolutePath(const std::string_view& path)
{
    std::string fullPath = ConvertHomeToFull(path);

    // fullPath가 절대 경로인지 확인, 절대 경로가 아니면 현재 작업 디렉토리를 앞에 붙여 절대 경로로 만듦
    if (!(fullPath.size() > 0 && fullPath[0] == '/'))
    {
        // 상대 경로: 현재 작업 디렉토리
        char cwdBuf[4096];
        if (getcwd(cwdBuf, sizeof(cwdBuf)))
        {
            std::string cwd(cwdBuf);
            if (cwd.back() != '/')
                cwd += '/';
            fullPath = cwd + fullPath;
        }
    }

    return fullPath;
}

std::string Directory::ConvertHomeToFull(const std::string_view& inPath)
{
    std::string path(inPath);

    if (path.empty() || path[0] != '~')
        return path;

    const char* home = std::getenv("HOME");

    if (home == nullptr)
        return std::string(path);

    std::string homePath(home);

    if (path.size() == 1)
        return homePath;

    // 확장된 경로 반환
    if (path.size() > 1 && path[1] == '/')
        return homePath + path.substr(1);

    return homePath + "/" + path.substr(1);
}

bool Directory::EnsureDirectoryExists(const std::string_view& path)
{
    if (path.empty())
        return false;

    // 절대 경로를 가져옴
    std::string fullPath = GetAbsolutePath(path);

    // 각 하위 경로를 순서대로 생성
    for (size_t i = 1; i < fullPath.size(); ++i)
    {
        // 해당 위치가 디렉토리 구분자가 아닌 경우 다음으로 이동
        if (fullPath[i] != '/')
            continue;

        std::string sub = fullPath.substr(0, i);

        if (sub.empty())
            continue;

        if (!HasDirExists(sub))
        {
            if (!MakeDir(sub))
                return false;
        }
    }

    // 마지막으로 전체 경로가 존재하지 않으면 생성
    if (!HasDirExists(fullPath))
    {
        if (!MakeDir(fullPath))
            return false;
    }

    return true;
}
