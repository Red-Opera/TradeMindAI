#include "Network.h"

#include <string>

size_t Network::WriteCallBack(void* contents, size_t size, size_t nmemb, std::string* output)
{
	size_t totalSize = size * nmemb;
	output->append((char*)contents, totalSize);

	return totalSize;
}
