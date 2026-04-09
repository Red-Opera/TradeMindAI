#include "Core/NewsAPI/NewsAPI.h"
#include "Utility/Coroutine/CoroutineManager.h"

#include <thread>

int main()
{
	CoroutineManager& corutineManager = CoroutineManager::GetInstance();
	NewsAPI& newsAPI = NewsAPI::GetInstance();	// 싱글톤 인스턴스 가져오기 (파일에서 뉴스 로드)

	corutineManager.Start(newsAPI.UpdateNews());

	while (true)
	{
		corutineManager.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
	}

	return 0;
}