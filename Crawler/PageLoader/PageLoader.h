#ifndef PAGE_LOADER_H
#define PAGE_LOADER_H

#include <string>
#include "../LoadResult/LoadResult.cpp"

class PageLoader
{
    static size_t GetData(char* buffer, size_t elementSize, size_t size, void* _);
    static bool IsHtml(char* text);
    
public:

    static LoadResult MakeRequest(const std::string& url, std::string& redirectedUrl);
};

#endif