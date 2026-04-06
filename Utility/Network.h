#pragma once

#include <iostream>
#include <string>

class Network
{
public:
	static size_t WriteCallBack(void* contents, size_t size, size_t nmemb, std::string* output);
};