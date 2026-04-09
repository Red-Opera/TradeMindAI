#pragma once

#include "Utility/Coroutine/IEnumerator.h"

#include <string>
#include <vector>
#include <unordered_set>

struct NewsData
{
	std::string title;
	std::string link;
	std::string date;
};

class NewsAPI
{
public:
	static NewsAPI& GetInstance();

	IEnumerator UpdateNews();
	const std::vector<NewsData>& GetStoredNews() const;	// 저장된 뉴스 목록 반환

	void ClearStoredNews();		// 저장된 뉴스 목록 초기화
	void SaveNews() const;		// 뉴스를 JSON 파일에 저장
	void LoadNews();			// JSON 파일에서 뉴스를 로드

private:
	NewsAPI();
	~NewsAPI();
	NewsAPI(const NewsAPI&) = delete;
	NewsAPI& operator=(const NewsAPI&) = delete;

	std::string GetRSS(const std::string& query);				// CURL을 사용하여 RSS 피드를 가져오는 메소드
	std::vector<NewsData> ParseRSS(const std::string& rssData);	// RSS 데이터를 파싱하여 뉴스 데이터를 추출하는 메소드
	bool HasNewsExists(const std::string& link) const;			// 뉴스가 이미 저장되어 있는지 확인

	// JSON 처리 헬퍼 메서드
	void ParseJsonNews(const std::string& jsonContent);
	std::string EscapeJsonString(const std::string& str) const;
	std::string UnescapeJsonString(const std::string& str) const;
	bool ExtractJsonField(const std::string& jsonStr, const std::string& fieldName, std::string& fieldValue) const;

	std::vector<NewsData> newsList;						// 중복이 제거된 뉴스 목록
	std::unordered_set<std::string> newsLinkSet;		// 빠른 중복 검사를 위한 link 해시 셋

	static constexpr const char* newsFileName = "NewsData.json";	// 뉴스 데이터 파일명
};