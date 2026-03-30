#pragma once

#include "WaitTime.h"

class WaitForSeconds : public WaitTime
{
public:
	WaitForSeconds(float sec);

	bool IsDone() const override;

private:
	float seconds;					// 대기할 시간을 저장하는 멤버 변수
};
