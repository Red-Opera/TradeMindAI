#include "Convert.h"

double Convert::JsonToDouble(const json& j, const std::string& key)
{
    try
    {
        if (!j.contains(key))
            return 0.0;

        if (j[key].is_string())
        {
            // JSON 값을 문자열로 가져옴
            std::string s = j[key].get<std::string>();

            // 빈 문자열인 경우 0.0 반환
            if (s.empty())
                return 0.0;

            // string 타입을 double로 변환
            return std::stod(s);
        }

        else if (j[key].is_number())
            return j[key].get<double>();
    }

    catch (...) {}

    return 0.0;
}

long Convert::JsonToLong(const json& j, const std::string& key)
{
    try
    {
        if (!j.contains(key))
            return 0;

        if (j[key].is_string())
        {
            std::string s = j[key].get<std::string>();

            if (s.empty())
                return 0;

            // string 타입을 long로 변환
            return std::stol(s);
        }

        else if (j[key].is_number_integer())
            return j[key].get<long>();
    }

    catch (...) {}

    return 0;
}

JsonData Convert::GetJsonData(const std::string_view& data)
{
    auto pos = data.find(':');

    if (pos == std::string::npos)
        return JsonData{ "", "" };

    JsonData result;

    result.key = std::string(data.substr(0, pos));
    result.value = std::string(data.substr(pos + 1));

    return result;
}

json Convert::ParseJsonString(const std::string& jsonStr)
{
    try
    {
        return json::parse(jsonStr);
    }
    catch (const std::exception&)
    {
        return json::object();
    }
}

std::string Convert::JsonToString(const json& j)
{
    try
    {
        return j.dump(2);
    }
    catch (const std::exception&)
    {
        return "";
    }
}

std::string Convert::EscapeJsonString(const std::string& str)
{
    std::string result;

    for (char c : str)
    {
        switch (c)
        {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c;
        }
    }

    return result;
}

std::string Convert::UnescapeJsonString(const std::string& str)
{
    std::string result;

    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '\\' && i + 1 < str.length())
        {
            switch (str[i + 1])
            {
            case '"': result += '"'; ++i; break;
            case '\\': result += '\\'; ++i; break;
            case 'b': result += '\b'; ++i; break;
            case 'f': result += '\f'; ++i; break;
            case 'n': result += '\n'; ++i; break;
            case 'r': result += '\r'; ++i; break;
            case 't': result += '\t'; ++i; break;
            default: result += str[i];
            }
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}