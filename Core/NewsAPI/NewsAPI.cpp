#include "NewsAPI.h"

#include "Utility/Coroutine/IEnumerator.h"

#include <iostream>

IEnumerator NewsAPI::UpdateNews()
{
	while (true)
	{
		std::cout << "Start\n";

		co_yield WaitForSeconds(1.0f);

		std::cout << "After 1 second\n";

		co_yield WaitForSeconds(2.0f);

		std::cout << "After 2 more seconds\n";
	}
}
