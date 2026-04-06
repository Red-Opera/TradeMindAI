#pragma once

#include <chrono>

class WaitTime
{
public:
	WaitTime();
	virtual ~WaitTime() = default;

	virtual bool IsDone() const;

protected:
	float GetElapsedSconds() const;						// 초보다 더 큰 단위를 사용할 때도 유용하게 사용하도록 초단위를 반환하는 메소드

	std::chrono::steady_clock::time_point startTime;    // 대기 시작 시간을 저장하는 멤버 변수
};