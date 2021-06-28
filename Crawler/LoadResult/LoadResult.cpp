#include "LoadResult.h"

LoadResult::LoadResult(const std::string& htmlDocument, int status)
{
    _htmlDocument = htmlDocument;
    _status = status;
}

int LoadResult::GetStatus()
{
    return _status;
}

std::string LoadResult::GetHtmlDocument()
{
    return _htmlDocument;
}