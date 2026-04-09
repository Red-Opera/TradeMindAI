#pragma once

#include <nlohmann/json.hpp>

#include <string_view>

using json = nlohmann::json;

struct JsonData
{
	std::string key;
	std::string value;
};

class Convert
{
public:
	static double JsonToDouble(const json& j, const std::string& key);
	static long JsonToLong(const json& j, const std::string& key);

	static JsonData GetJsonData(const std::string_view& data);				// 키:값 쌍을 JsonData 구조체로 반환

	// JSON 파싱 및 생성 헬퍼 메서드
	static json ParseJsonString(const std::string& jsonStr);
	static std::string JsonToString(const json& j);
	static std::string EscapeJsonString(const std::string& str);
	static std::string UnescapeJsonString(const std::string& str);
};

