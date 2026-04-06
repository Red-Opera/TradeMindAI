#pragma once

#include "Utility/Coroutine/IEnumerator.h"

#include <string>
#include <vector>

struct NewsData
{
	std::string title;
	std::string link;
	std::string date;
};

class NewsAPI
{
public:
	static IEnumerator UpdateNews();

private:
	static std::string GetRSS(const std::string& query);				// CURL을 사용하여 RSS 피드를 가져오는 메소드
	static std::vector<NewsData> ParseRSS(const std::string& rssData);	// RSS 데이터를 파싱하여 뉴스 데이터를 추출하는 메소드
};