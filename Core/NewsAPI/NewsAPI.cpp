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
	// CURL 초기화
	CURL* curl = curl_easy_init();

	if (curl == nullptr)
	{
		std::cerr << "CURL 초기화 실패" << std::endl;
		return "";
	}

	// 쿼리를 URL 인코딩
	char* encodedQuery = curl_easy_escape(curl, query.c_str(), (int)query.length());

	if (encodedQuery == nullptr)
	{
		std::cerr << "쿼리 인코딩 실패" << std::endl;
		curl_easy_cleanup(curl);
		return "";
	}

	// 인코딩된 쿼리로 URL 구성
	std::string url = "https://news.google.com/rss/search?q=" + std::string(encodedQuery) + "&hl=ko&gl=KR&ceid=KR:ko";
	curl_free(encodedQuery);

	std::cout << "요청 URL: " << url << std::endl;

	std::string readBuffer;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());						// 요청할 URL 설정
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Network::WriteCallBack);	// 응답 데이터를 처리할 콜백 함수 설정
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);					// 콜백 함수에 전달할 데이터 버퍼 설정
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);						// SSL 검증 비활성화 (테스트용)
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);						// 호스트 검증 비활성화 (테스트용)
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");	// User-Agent 설정
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);							// 타임아웃 10초

	CURLcode res = curl_easy_perform(curl);	// 요청 수행

	// 요청 실패 시 에러 메시지 출력
	if (res != CURLE_OK)
	{
		std::cerr << "CURL 요청 실패. 에러 코드: " << res << std::endl;
		std::cerr << "에러 메시지: " << curl_easy_strerror(res) << std::endl;
		curl_easy_cleanup(curl);

		return "";
	}

	long responseCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	std::cout << "HTTP 응답 코드: " << responseCode << std::endl;
	std::cout << "수신한 데이터 크기: " << readBuffer.length() << " bytes" << std::endl;

	curl_easy_cleanup(curl);				// CURL 리소스 해제

	// 응답 데이터 반환
	return readBuffer;	
}

std::vector<NewsData> NewsAPI::ParseRSS(const std::string& rssData)
{
	// News 데이터를 저장할 벡터
	std::vector<NewsData> newsList;

	// 수신한 데이터 확인
	if (rssData.empty())
	{
		std::cerr << "수신한 RSS 데이터가 비어있습니다" << std::endl;

		return newsList;
	}

	std::cout << "수신한 RSS 데이터 크기: " << rssData.length() << " bytes" << std::endl;

	XMLDocument document;
	XMLError parseResult = document.Parse(rssData.c_str());
	
	// XML 파싱이 실패한 경우 에러 메시지 출력
	if (parseResult != XML_SUCCESS)
	{
		std::cerr << "RSS 파싱 실패. 에러 코드: " << parseResult << std::endl;
		std::cerr << "에러 메시지: " << document.ErrorStr() << std::endl;
		std::cerr << "처음 500 characters: " << rssData.substr(0, 500) << std::endl;

		return newsList;
	}

	XMLElement* rssElement = document.FirstChildElement("rss");

	if (rssElement == nullptr)
	{
		std::cerr << "RSS 요소를 찾을 수 없습니다" << std::endl;
		// 첫 번째 요소가 무엇인지 확인
		XMLElement* root = document.RootElement();
		if (root != nullptr)
		{
			std::cerr << "실제 루트 요소: " << root->Name() << std::endl;
		}
		return newsList;
	}

	XMLElement* channelElement = rssElement->FirstChildElement("channel");

	if (channelElement == nullptr)
	{
		std::cerr << "channel 요소를 찾을 수 없습니다" << std::endl;

		return newsList;
	}

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
