#ifndef CRAWLER_H
#define CRAWLER_H

#include "../PageLoader/PageLoader.cpp"
#include "../ParsedData/ParsedData.cpp"
#include "../DataBase/DataBase.cpp"

#include <fstream>
#include <gumbo.h>
#include <assert.h>
#include <vector>
#include <set>
#include <algorithm>
#include <queue>

class Crawler
{
    std::string _domain;
    std::string _startUrl;
    std::string _currentUrl;
    std::string _redirectedUrl;
    std::set<std::string> _visitedUrls;
    
    std::vector<std::string> _ends;
    std::vector<std::string> _starts;
    
    std::vector<std::string> GetLinks(const std::string& document);
    bool IsValidUrl(std::string& url);

    static std::string ToUpper(const std::string& str);
    static std::string RemoveLastDir(const std::string& str);
    static std::string GetText(GumboNode* node, std::string& title, std::string& description);
    static std::string GetDomain(std::string& url);
    static void GetLinks(GumboNode* node, std::set<std::string>& links);
    std::pair<std::set<std::string>, ParsedData> Parse(const std::string& document);

public:

    Crawler(const std::string& startUrl);

    std::vector<ParsedData> Begin();
};

#endif