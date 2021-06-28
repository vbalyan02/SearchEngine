#include "PageLoader.h"

size_t PageLoader::GetData(char* buffer, size_t elementSize, size_t size, void* string)
{
    std::string* intermediateDocument = (std::string*)string;
    *intermediateDocument += (std::string(buffer, elementSize * size));

    return elementSize * size;
}

bool PageLoader::IsHtml(char* text)
{
    if(text == nullptr)
        return false;

    std::string str(text);

    if(str.substr(0, 9) == "text/html" || str.substr(0, 10) == "text/plain")
        return true;

    return false;
}

LoadResult PageLoader::MakeRequest(const std::string& url, std::string& redirectedUrl)
{
    CURL* curl = curl_easy_init();
    
    if(curl == nullptr)
        return LoadResult("", -1);

    std::string* intermediateDocument = new std::string();

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept:text/html");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)intermediateDocument);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 4);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, headers);

    CURLcode response = curl_easy_perform(curl);
    
    char* type, redirUrl;
    long responseCode;
    
    curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL , &redirUrl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &type);
    

    if(response != CURLE_OK || !IsHtml(type))
        return LoadResult("", -1);

    curl_easy_cleanup(curl);

    return LoadResult(*intermediateDocument, responseCode);
}