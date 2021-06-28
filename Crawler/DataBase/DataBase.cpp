#include "DataBase.h"

sql::Connection* DataBase::_connection;
sql::Driver* DataBase::_driver;
sql::Statement* DataBase::_statement;
sql::PreparedStatement* DataBase::_preparedStatement;

void DataBase::Init(const std::string& host, const std::string& username, const std::string& password, const std::string& schema)
{
    try
    {
        DataBase::_driver = get_driver_instance();
        DataBase::_connection = _driver->connect(host, username, password);
        DataBase::_connection->setSchema(schema);
        DataBase::_statement = _connection->createStatement();
    }
    catch (sql::SQLException& e)
    {
        std::cerr << e.what();
    }
}

sql::ResultSet* DataBase::MakeRequest(const std::string& query)
{
    try
    {
        return _statement->executeQuery(query);
    }
    catch (sql::SQLException& e)
    {
        std::cerr << e.what() << '\n';
        
        return nullptr;
    }
}

sql::ResultSet* DataBase::MakeRequest(const std::string& preparedStatement, const std::vector<std::string>& types, const std::vector<std::string>& values)
{
    try
    {
        _preparedStatement = _connection->prepareStatement(preparedStatement);
        
        for(int i = 0; i < types.size(); ++i)
        {
            if(types[i] == "int")
                _preparedStatement->setInt(i + 1, std::stoi(values[i]));
            else if(types[i] == "string")
                _preparedStatement->setString(i + 1, values[i]);
        }

        return _preparedStatement->executeQuery();
    }
    catch(sql::SQLException& e)
    {
        std::cout << e.what();
        return nullptr;
    }
}