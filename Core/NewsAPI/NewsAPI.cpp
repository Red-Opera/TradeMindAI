#include "NewsAPI.h"

#include "Utility/Coroutine/IEnumerator.h"
#include "Utility/Network.h"

#include "tinyxml2.h"

#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>

using namespace tinyxml2;

IEnumerator NewsAPI::UpdateNews()
{
	while (true)
	{
		std::string query = "stock OR market OR economy when:1d";	// 최근 24시간 동안의 뉴스 검색
		std::string xmlData = GetRSS(query);
		std::vector<NewsData> newsList = ParseRSS(xmlData);

		for (const auto& news : newsList)
		{
			std::cout << "Title: " << news.title << std::endl;
			std::cout << "Link: " << news.link << std::endl;
			std::cout << "Date: " << news.date << std::endl;
			std::cout << "-----------------------------" << std::endl;
		}

		co_yield WaitForSeconds(2.0f);
	}
}

std::string NewsAPI::GetRSS(const std::string& query)
{
	// 쿼리 문자열을 URL 인코딩
	std::string url = "https://news.google.com/rss/search?q=" + query + "&hl=ko&gl=KR&ceid=KR:ko";

	// CURL 초기화
	CURL* curl = curl_easy_init();

	if (curl == nullptr)
	{
		std::cerr << "CURL 초기화 실패" << std::endl;
		return "";
	}

	std::string readBuffer;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());						// 요청할 URL 설정
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Network::WriteCallBack);	// 응답 데이터를 처리할 콜백 함수 설정
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);					// 콜백 함수에 전달할 데이터 버퍼 설정

	CURLcode res = curl_easy_perform(curl);	// 요청 수행
	curl_easy_cleanup(curl);				// CURL 리소스 해제

	// 응답 데이터 반환
	return readBuffer;	
}

std::vector<NewsData> NewsAPI::ParseRSS(const std::string& rssData)
{
	// News 데이터를 저장할 벡터
	std::vector<NewsData> newsList;

	XMLDocument document;
	document.Parse(rssData.c_str());

	XMLElement* channelElement = document.FirstChildElement("rss")->FirstChildElement("channel");
	XMLElement* itemElement = channelElement->FirstChildElement("item");

	while (itemElement != nullptr)
	{
		NewsData news;

		XMLElement* titleElement = itemElement->FirstChildElement("title");
		XMLElement* linkElement = itemElement->FirstChildElement("link");
		XMLElement* dateElement = itemElement->FirstChildElement("pubDate");

		if (titleElement != nullptr)
			news.title = titleElement->GetText();

		if (linkElement != nullptr)
			news.link = linkElement->GetText();

		if (dateElement != nullptr)
			news.date = dateElement->GetText();

		newsList.push_back(news);
		itemElement = itemElement->NextSiblingElement("item");
	}

	return newsList;
}
