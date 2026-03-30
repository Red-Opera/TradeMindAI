#include "CoroutineManager.h"

CoroutineManager& CoroutineManager::GetInstance()
{
	static CoroutineManager instance;

	return instance;
}

void CoroutineManager::Start(IEnumerator&& routine)
{
	coroutines.emplace_back(std::move(routine));
}

void CoroutineManager::Update()
{
	for (auto iter = coroutines.begin(); iter != coroutines.end(); )
	{
		if (!iter->MoveNext())
			iter = coroutines.erase(iter);

		else
			++iter;
	}
}