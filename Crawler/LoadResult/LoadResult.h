#ifndef LOAD_RESULT_H
#define LOAD_RESULT_H

#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>

#include <iostream>
#include <string>
#include <vector>


class LoadResult
{
    int _status;
    std::string _htmlDocument;

public:

    LoadResult() = default;

    LoadResult(const std::string& htmlDocument, int status);

    std::string GetHtmlDocument();

    int GetStatus();
};


#endif