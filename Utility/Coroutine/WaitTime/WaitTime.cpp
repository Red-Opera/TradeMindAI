#include "WaitTime.h"

WaitTime::WaitTime() : startTime(std::chrono::steady_clock::now())
{

}

bool WaitTime::IsDone() const
{
	return false;
}

float WaitTime::GetElapsedSconds() const
{
	auto now = std::chrono::steady_clock::now();

	return std::chrono::duration<float>(now - startTime).count();
}
