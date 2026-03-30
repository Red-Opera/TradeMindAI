#include "WaitForSeconds.h"

WaitForSeconds::WaitForSeconds(float sec) : WaitTime(), seconds(sec)
{

}

bool WaitForSeconds::IsDone() const
{
    auto now = std::chrono::steady_clock::now();

    // 지정한 대기 시간이 경과했는지 확인
    return std::chrono::duration<float>(now - startTime).count() >= seconds;
}
