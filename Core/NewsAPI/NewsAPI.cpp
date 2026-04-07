#include "NewsAPI.h"

#include "../Log.h"
#include "Utility/Coroutine/IEnumerator.h"
#include "Utility/Network.h"

#include "tinyxml2.h"

#include <sstream>
#include <vector>
#include <string>
#include <curl/curl.h>

using namespace tinyxml2;

IEnumerator NewsAPI::UpdateNews()
{
	Log& log = Log::GetInstance();

	while (true)
	{
		std::string query = "stock OR market OR economy when:1d";	// 최근 24시간 동안의 뉴스 검색
		std::string xmlData = GetRSS(query);
		std::vector<NewsData> newsList = ParseRSS(xmlData);

		for (const auto& news : newsList)
		{
			log.Output(LogLevel::INFO, ("Title: " + news.title).c_str());
			log.Output(LogLevel::INFO, ("Link: " + news.link).c_str());
			log.Output(LogLevel::INFO, ("Date: " + news.date).c_str());
			log.Output(LogLevel::INFO, "-----------------------------");
		}

		co_yield WaitForSeconds(2.0f);
	}
}

std::string NewsAPI::GetRSS(const std::string& query)
{
	Log& log = Log::GetInstance();

	// CURL 초기화
	CURL* curl = curl_easy_init();

	if (curl == nullptr)
	{
		log.Output(LogLevel::ERROR, "CURL 초기화 실패");
		return "";
	}

	// 쿼리를 URL 인코딩
	char* encodedQuery = curl_easy_escape(curl, query.c_str(), (int)query.length());

	if (encodedQuery == nullptr)
	{
		log.Output(LogLevel::ERROR, "쿼리 인코딩 실패");
		curl_easy_cleanup(curl);
		return "";
	}

	// 인코딩된 쿼리로 URL 구성
	std::string url = "https://news.google.com/rss/search?q=" + std::string(encodedQuery) + "&hl=ko&gl=KR&ceid=KR:ko";
	curl_free(encodedQuery);

	log.Output(LogLevel::INFO, ("요청 URL: " + url).c_str());

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
		std::ostringstream errorCode;
		errorCode << "CURL 요청 실패. 에러 코드: " << static_cast<int>(res);
		log.Output(LogLevel::ERROR, errorCode.str().c_str());
		log.Output(LogLevel::ERROR, (std::string("에러 메시지: ") + curl_easy_strerror(res)).c_str());
		curl_easy_cleanup(curl);

		return "";
	}

	long responseCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	{
		std::ostringstream responseLog;
		responseLog << "HTTP 응답 코드: " << responseCode;
		log.Output(LogLevel::INFO, responseLog.str().c_str());
	}
	{
		std::ostringstream sizeLog;
		sizeLog << "수신한 데이터 크기: " << readBuffer.length() << " bytes";
		log.Output(LogLevel::INFO, sizeLog.str().c_str());
	}

	curl_easy_cleanup(curl);				// CURL 리소스 해제

	// 응답 데이터 반환
	return readBuffer;	
}

std::vector<NewsData> NewsAPI::ParseRSS(const std::string& rssData)
{
	Log& log = Log::GetInstance();

	// News 데이터를 저장할 벡터
	std::vector<NewsData> newsList;

	// 수신한 데이터 확인
	if (rssData.empty())
	{
		log.Output(LogLevel::WARNING, "수신한 RSS 데이터가 비어있습니다");

		return newsList;
	}

	{
		std::ostringstream rssSizeLog;
		rssSizeLog << "수신한 RSS 데이터 크기: " << rssData.length() << " bytes";
		log.Output(LogLevel::INFO, rssSizeLog.str().c_str());
	}

	XMLDocument document;
	XMLError parseResult = document.Parse(rssData.c_str());
	
	// XML 파싱이 실패한 경우 에러 메시지 출력
	if (parseResult != XML_SUCCESS)
	{
		std::ostringstream parseError;
		parseError << "RSS 파싱 실패. 에러 코드: " << static_cast<int>(parseResult);
		log.Output(LogLevel::ERROR, parseError.str().c_str());
		log.Output(LogLevel::ERROR, (std::string("에러 메시지: ") + document.ErrorStr()).c_str());
		log.Output(LogLevel::ERROR, (std::string("처음 500 characters: ") + rssData.substr(0, 500)).c_str());

		return newsList;
	}

	XMLElement* rssElement = document.FirstChildElement("rss");

	if (rssElement == nullptr)
	{
		log.Output(LogLevel::ERROR, "RSS 요소를 찾을 수 없습니다");
		// 첫 번째 요소가 무엇인지 확인
		XMLElement* root = document.RootElement();
		if (root != nullptr)
		{
			log.Output(LogLevel::ERROR, (std::string("실제 루트 요소: ") + root->Name()).c_str());
		}
		return newsList;
	}

	XMLElement* channelElement = rssElement->FirstChildElement("channel");

	if (channelElement == nullptr)
	{
		log.Output(LogLevel::ERROR, "channel 요소를 찾을 수 없습니다");

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
