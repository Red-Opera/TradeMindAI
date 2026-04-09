#include "NewsAPI.h"

#include "Core/Log.h"
#include "Core/Config.h"
#include "Utility/Coroutine/IEnumerator.h"
#include "Utility/Network.h"
#include "Utility/Directory.h"
#include "Utility/Convert.h"

#include "tinyxml2.h"

#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <algorithm>

using namespace tinyxml2;

// 싱글톤 인스턴스
NewsAPI& NewsAPI::GetInstance()
{
	static NewsAPI instance;

	return instance;
}

// 초기화 시 파일에서 뉴스 로드
NewsAPI::NewsAPI()
{
	LoadNews();
}

// 소멸자
NewsAPI::~NewsAPI()
{
	SaveNews();
}

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
			// 중복이 아닌 경우만 저장
			if (!HasNewsExists(news.link))
			{
				newsList.push_back(news);
				newsLinkSet.insert(news.link);

				log.Output(LogLevel::INFO, ("새로운 뉴스 추가 - Title: " + news.title).c_str());
				log.Output(LogLevel::INFO, ("Link: " + news.link).c_str());
				log.Output(LogLevel::INFO, ("Date: " + news.date).c_str());
				log.Output(LogLevel::INFO, "-----------------------------");
			}
		}

		// 업데이트된 뉴스를 파일에 저장
		SaveNews();

		co_yield WaitForSeconds(2.0f);
	}
}

bool NewsAPI::HasNewsExists(const std::string& link) const
{
	return newsLinkSet.find(link) != newsLinkSet.end();
}

const std::vector<NewsData>& NewsAPI::GetStoredNews() const
{
	return newsList;
}

void NewsAPI::ClearStoredNews()
{
	newsList.clear();
	newsLinkSet.clear();

	SaveNews();
}

void NewsAPI::SaveNews() const
{
	Log& log = Log::GetInstance();

	// 파일 경로 생성
	std::string filePath = Directory::ConvertHomeToFull(std::string(Config::rootPath) + "/" + newsFileName);

	// 디렉토리가 없으면 생성
	size_t lastSlash = filePath.find_last_of("/\\");

	if (lastSlash != std::string::npos)
	{
		std::string dir = filePath.substr(0, lastSlash);
		Directory::EnsureDirectoryExists(dir);
	}

	std::ofstream file(filePath);

	if (!file.is_open())
	{
		log.Output(LogLevel::ERROR, ("뉴스 파일 생성 실패 : " + filePath).c_str());

		return;
	}

	// JSON 형식으로 저장
	json newsJson = json::array();

	for (const auto& news : newsList)
		newsJson.push_back({ {"title", news.title}, {"link", news.link}, {"date", news.date} });

	json root;
	root["news"] = newsJson;

	file << root.dump(2);
	file.close();

	std::ostringstream logMessage;
	logMessage << "뉴스 데이터 저장됨: " << filePath << " (" << newsList.size() << "개)";

	log.Output(LogLevel::INFO, logMessage.str().c_str());
}

void NewsAPI::LoadNews()
{
	Log& log = Log::GetInstance();

	std::string filePath = Directory::ConvertHomeToFull(std::string(Config::rootPath) + "/" + newsFileName);

	std::ifstream file(filePath);

	if (!file.is_open())
	{
		log.Output(LogLevel::WARNING, ("뉴스 파일을 찾을 수 없음 : " + filePath).c_str());

		return;
	}

	std::string line;
	std::string jsonContent;

	while (std::getline(file, line))
		jsonContent += line;

	file.close();

	// 간단한 JSON 파싱 (문자열 기반)
	ParseJsonNews(jsonContent);

	std::ostringstream logMsg;
	logMsg << "뉴스 데이터 로드됨 : " << filePath << " (" << newsList.size() << "개)";
	log.Output(LogLevel::INFO, logMsg.str().c_str());
}

void NewsAPI::ParseJsonNews(const std::string& jsonContent)
{
	Log& log = Log::GetInstance();

	newsList.clear();
	newsLinkSet.clear();

	try
	{
		json jsonObject = Convert::ParseJsonString(jsonContent);

		if (!jsonObject.contains("news") || !jsonObject["news"].is_array())
		{
			log.Output(LogLevel::WARNING, "JSON 형식이 올바르지 않습니다: 'news' 배열이 없습니다");

			return;
		}

		// JSON 배열에서 뉴스 데이터 추출
		for (const auto& item : jsonObject["news"])
		{
			NewsData news;
			news.title = item.value("title", "");
			news.link = item.value("link", "");
			news.date = item.value("date", "");

			if (news.link.empty())
				continue;

			newsList.push_back(news);
			newsLinkSet.insert(news.link);
		}
	}

	catch (const std::exception& e)
	{
		log.Output(LogLevel::WARNING, (std::string("JSON 파싱 오류: ") + e.what()).c_str());
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
