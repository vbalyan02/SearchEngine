#include "Crawler.h"

std::string Crawler::ToUpper(const std::string& str)
{
    std::string ret;

    for(int i = 0; i < str.size(); ++i)
    {
        if(str[i] < 'A')
            ret.push_back(str[i] + 26);
        else
            ret.push_back(str[i]);
    }

    return ret;
}

std::string Crawler::RemoveLastDir(const std::string& str)
{
    for(int i = str.size() - 2; i >= 0; --i)
        if(str[i] == '/')
            return str.substr(0, i);

    return "";
}

bool Crawler::IsValidUrl(std::string& url)
{
    if(url.size() == 0 || url[0] == '#')
        return false;
    
    if(url.find("http://") == 0 || url.find("https://") == 0)
        return true;

    if(url.find(':') != std::string::npos)
        return false;

    for(int i = 0; i < _ends.size(); ++i)
    {
        int pos = url.size() - _ends[i].size();

        if(pos <= 0 || _ends[i].size() + pos > url.size())
            continue;

        std::string sub = url.substr(pos, _ends[i].size());

        if(ToUpper(sub) == ToUpper(_ends[i]))
            return false;
    }

    if(url[0] == '/')
    {
        int domainPos = _currentUrl.find(_domain);

        int slash = _currentUrl.find('/', domainPos);

        int pos;

        if(slash == std::string::npos)
            pos = _currentUrl.size() - 1;
        else
            pos = slash;
        
        url = _currentUrl.substr(0, pos) + url;
    }
    else
    {
        if(_currentUrl.back() == '/')
            url = _currentUrl + url;
        else
            url = RemoveLastDir(_currentUrl) + url;
    }

    return true;
}

void Crawler::GetLinks(GumboNode* node, std::set<std::string>& links) 
{
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    GumboAttribute* href;

    if (node->v.element.tag == GUMBO_TAG_A && (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) 
    {
        std::string temp(href->value);

        links.insert(temp);
    }

    GumboVector* children = &node->v.element.children;

    for (int i = 0; i < children->length; ++i)
    {
        GetLinks((GumboNode*)(children->data[i]), links);
    }
}

std::string Crawler::GetText(GumboNode* node, std::string& title, std::string& description) 
{
    if (node->type == GUMBO_NODE_TEXT)
        return std::string(node->v.text.text);

    else if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag != GUMBO_TAG_SCRIPT && node->v.element.tag != GUMBO_TAG_STYLE) 
    {
        if(node->v.element.tag == GUMBO_TAG_META)
        {  
            GumboAttribute* desc = gumbo_get_attribute(&node->v.element.attributes, "name");
            
            if(desc != nullptr && std::string(desc->value) == "description")
                    description = std::string(gumbo_get_attribute(&node->v.element.attributes, "content")->value);
        }
        if(node->v.element.tag == GUMBO_TAG_TITLE)
        {
            GumboNode* title_text = (GumboNode*) node->v.element.children.data[0];
            title = std::string(title_text->v.text.text);
        }


        std::string contents = "";
        GumboVector* children = &node->v.element.children;
        
        for (int i = 0; i < children->length; ++i)
        {
            std::string text = GetText((GumboNode*) children->data[i], title, description);

            if (i != 0 && !text.empty())
                contents.append(" ");

            contents.append(text);
        }

        return contents;
    }
    else
        return "";
}

std::string Crawler::GetDomain(std::string& url)
{
    int start = url.find(".");

    if(start == std::string::npos)
        return "error";
    
    ++start;
    
    int end = url.find('.', start);

    int slash = url.find('/', start);

    if(end == std::string::npos || (end > slash && slash != std::string::npos))
    {
        end = start - 1;
        start = url.find('/', 0) + 2;
    }

    std::string sub = url.substr(start, end - start);

    return sub;
}

Crawler::Crawler(const std::string& startUrl)
{
    _startUrl = startUrl;
    _ends = std::vector<std::string>{".doc", ".png", ".jpg", ".jpeg", ".pdf", ".djvu"};
    _starts = std::vector<std::string>{"mailto", "tel"};
}

std::pair<std::set<std::string>, ParsedData> Crawler::Parse(const std::string& document)
{
    std::set<std::string> links, ret;
    std::string text, link, title, description;

    GumboOutput* output = gumbo_parse_with_options(&kGumboDefaultOptions, document.c_str(), document.size());
    GumboNode* node = output->root;

    GetLinks(node, links);
    text = GetText(node, title, description);
    
    for(auto it = links.begin(); it != links.end(); ++it)
    {
        link = *it;

        if(IsValidUrl(link) && GetDomain(link) == _domain && _visitedUrls.insert(link).second)
            ret.insert(link);
    }

    gumbo_destroy_output(&kGumboDefaultOptions, output);
    
    return std::pair<std::set<std::string>, ParsedData>(ret, ParsedData(_currentUrl, title, description, text, _domain));
}

std::vector<ParsedData> Crawler::Begin()
{
    int count = 0;
    std::vector<ParsedData> ret;
    std::queue<std::string> linkQueue;
    std::vector<std::string> links;

    LoadResult result;

    int status;
    std::string link;

    linkQueue.push(_startUrl);
    _visitedUrls.insert(_startUrl);
    _domain = GetDomain(_startUrl);

    while(linkQueue.size() != 0)
    {
        _currentUrl = linkQueue.front();

        linkQueue.pop();

        result = PageLoader::MakeRequest(_currentUrl, _redirectedUrl);

        if(_redirectedUrl != "" && _redirectedUrl != _currentUrl && !_visitedUrls.insert(_redirectedUrl).second)
            continue;

        status = result.GetStatus();
        
        if(status < 200 || status > 399)
            continue;
        
        std::cout << ++count << " Size: " << linkQueue.size() << ' ' + _currentUrl << '\n';
        
        std::pair<std::set<std::string>, ParsedData> data = Parse(result.GetHtmlDocument());

        ret.push_back(data.second);

        for(auto it = data.first.begin(); it != data.first.end(); ++it)
            linkQueue.push(*it);
    }

    return ret;
}

int main()
{
    int count = 0, save;
    std::string startUrl;
    
    std::vector<std::string> types = {"string", "string", "string", "string", "string"}, values;
    std::string request = "insert into WebsiteData.WebsiteData values(?,?,?,?,?)";

    DataBase::Init("tcp://127.0.0.1:3306", "root", "154215", "WebsiteData");
    
    std::cout << "Enter urls to start with. Type \"quit\" to end the input\n";

    while(true)
    {
        save = count;

        std::cin >> startUrl;

        if(startUrl == "quit")
            break;

        Crawler crawler(startUrl);

        std::vector<ParsedData> parsedData = crawler.Begin();
        
        
        for(int i = 0; i < parsedData.size(); ++i)
        {
            values = {parsedData[i].GetUrl(), parsedData[i].GetTitle(), parsedData[i].GetDescription(), parsedData[i].GetText(), parsedData[i].GetDomain()};

            if(DataBase::MakeRequest(request, types, values) != nullptr)
                ++count;
        }

        std::cout << "\n\n**Finished indexing \"" + startUrl + "\". Indexed " << count - save << " new webpages.**\n\n";
    }

    std::cout << "\n\nProgram finished successfully. " << count << " new webpages have been indexed.\n";
}
