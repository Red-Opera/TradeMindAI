#include "Core/NewsAPI/NewsAPI.h"
#include "Utility/Coroutine/CoroutineManager.h"

#include <thread>

int main()
{
	CoroutineManager& corutineManager = CoroutineManager::GetInstance();
	corutineManager.Start(NewsAPI::UpdateNews());

	while (true)
	{
		corutineManager.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
	}

	return 0;
}