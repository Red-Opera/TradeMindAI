#pragma once

#include "IEnumerator.h"

#include <vector>
#include <memory>

class CoroutineManager
{
public:
	static CoroutineManager& GetInstance();

	void Start(IEnumerator&& routine);
	void Update();

private:
	std::vector<IEnumerator> coroutines;
};