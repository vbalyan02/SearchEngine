#include "../Crawler/DataBase/DataBase.cpp"
#include "../Crawler/ParsedData/ParsedData.cpp"
#include <nuspell/dictionary.hxx>
#include <nuspell/finder.hxx>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <algorithm>
#include <thread>
#include <set>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <sstream>
#include <fstream>

using boost::property_tree::ptree;
using tcp = boost::asio::ip::tcp;

void InitDatabase()
{
    DataBase::Init("tcp://127.0.0.1:3306", "root", "154215", "WebsiteData");
    
    try
    {
        DataBase::MakeRequest("ALTER TABLE WebsiteData ADD FULLTEXT(title);");
        DataBase::MakeRequest("ALTER TABLE WebsiteData ADD FULLTEXT(description);");
        DataBase::MakeRequest("ALTER TABLE WebsiteData ADD FULLTEXT(text);");
        DataBase::MakeRequest("ALTER TABLE WebsiteData ADD FULLTEXT(url);");
    }
    catch(sql::SQLException& e)
    {
        std::cerr << e.what() << '\n';
    }
}

std::string CheckSpelling(const std::string& word)
{
    std::string ret = "";
   	auto dict_list = std::vector<std::pair<std::string, std::string>>();
	nuspell::search_default_dirs_for_dicts(dict_list);
	auto dict_name_and_path = nuspell::find_dictionary(dict_list, "en_US");

	if (dict_name_and_path == end(dict_list))
		return "";
	
    auto& dict_path = dict_name_and_path->second;

	auto dict = nuspell::Dictionary::load_from_path(dict_path);

	std::vector<std::string> suggestions;

    if (dict.spell(word)) 
        return "+";

    dict.suggest(word, suggestions);
    
    if (suggestions.empty())
        return "-";
    
    for (auto& suggestion : suggestions)
        ret += suggestion + '\n';


    return ret;
}

ParsedData SetToParsedData(sql::ResultSet* set)
{
    std::string url = set->getString("url");
    std::string title = set->getString("title");
    std::string description = set->getString("description");
    std::string text = set->getString("text");
    std::string domain = set->getString("domain");

    return ParsedData(url, title, description, text, domain);
}

std::string to_json(ParsedData data)
{
    ptree out;
    out.put("url", data.GetUrl());
    out.put("title", data.GetTitle());
    out.put("description", data.GetDescription());

    std::ostringstream oss;
    boost::property_tree::write_json(oss, out);

    return oss.str();
}

std::string Search(const std::string& query)
{
    std::string ret = "", url;
    std::set<std::string> urls;
    bool found = false;
    
    sql::ResultSet* titleSet = DataBase::MakeRequest("SELECT * FROM WebsiteData WHERE Match(title) Against(\'" + query + "\');");
    sql::ResultSet* descriptionSet = DataBase::MakeRequest("SELECT * FROM WebsiteData WHERE Match(description) Against(\'" + query + "\');");
    sql::ResultSet* textSet = DataBase::MakeRequest("SELECT * FROM WebsiteData WHERE Match(text) Against(\'" + query + "\');");

    while(titleSet->next())
    {
        found = true;
        
        url = titleSet->getString("url");

        if(urls.insert(url).second)
            ret += to_json(SetToParsedData(titleSet)) + ',';//ret += url + '\n';
    }
    while(descriptionSet->next())
    {
        found = true;
        
        url = descriptionSet->getString("url");

        if(urls.insert(url).second)
            ret += to_json(SetToParsedData(descriptionSet)) + ',';//ret += url + '\n';
    }
    while(textSet->next())
    {   
        found = true;

        url = textSet->getString("url");

        if(urls.insert(url).second)
            ret += to_json(SetToParsedData(textSet)) + ',';//ret += url + '\n';
    }
    
    if(!found)
        ret = "Nothing found\n";

    return ret;
}



int main()
{
    InitDatabase();
    
    auto const address = boost::asio::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(std::atoi("18909"));
    boost::asio::io_context ioc{1};

    boost::asio::ip::tcp::acceptor acceptor{ioc, {address, port}};

    while(1)
    {
        tcp::socket socket{ioc};
        acceptor.accept(socket);

        std::cout << "socket accepted" << "\n";

        std::thread{ [q = std::move(socket)] () mutable 
        {
            boost::beast::websocket::stream<tcp::socket> ws {std::move(q)};
            ws.accept();

            while(true)
            {
                try
                {
                    boost::beast::flat_buffer buffer;

                    ws.read(buffer);

                    std::string query = boost::beast::buffers_to_string(buffer.data());

                    std::ofstream outp ("json output");
                    
                    std::string searchResult = Search(std::string(query));

                    std::string resultt = "{\n \"data\":[\n" + searchResult.substr(0, searchResult.size() - 2) + "\n]}";//= "{\ndata : [\n" + Search(std::string(query)) + "\n]\n}";

                    outp << resultt;

                    ws.write(boost::asio::buffer(std::basic_string<char>(resultt)));

                    auto out = boost::beast::buffers_to_string(buffer.cdata());
                    std::cout << out << "\n";
                }
                catch (boost::beast::system_error const& e)
                {
                    if(e.code() != boost::beast::websocket::error::closed)
                    {
                        std::cout << "Error catch: " << e.code().message() << "\n";
                        break;
                    }
                }
            }

        }}.detach();
    }
}