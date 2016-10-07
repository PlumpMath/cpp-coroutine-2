//
// Created by linyilong3 on 16/9/30.
//

#include "asioRedis.h"

#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>

static const char OK = '+';
static const char ERROR = '-';
static const char MSG_LINE = '*';
static const char NEXT_LINE = '$';
static const char INT_VAL = ':';


std::string _formatRedisCommand( const std::string &command )
{
    std::string ret;

    std::vector< std::string > commands;
    boost::split( commands, command, boost::is_any_of(" "));
    boost::format fmt = boost::format("*%d\r\n") % commands.size();
    ret += fmt.str();

    for ( auto iter = commands.begin(); iter != commands.end(); ++iter )
    {
        fmt = boost::format("$%d\r\n%s\r\n") % iter->size() % *iter;
        ret += fmt.str();
    }

    return std::move(ret);
}

std::string _getline( std::istream &is )
{
    std::string s;
    std::getline(is, s, '\n');

    s.erase( s.size() - 1 );

    return std::move(s);
}

int __getlineToInt(std::istream &is)
{
    std::string s;
    std::getline(is, s, '\n');

    s.erase( s.size() - 1 );

    return atoi(s.data());
}

void asioRedis::disconnect()
{
    _connection.close();
}

redisReply asioRedis::async_command(const std::string &command, const boost::asio::yield_context &yield)
{
    //todo: IO操作比较多,可以进行优化
    std::string redis_cmd = _formatRedisCommand(command);
    _connection.async_write_some( boost::asio::buffer(redis_cmd), yield);

    boost::asio::streambuf buf;
    boost::asio::async_read_until( _connection, buf, "\r\n", yield);

    std::istream is(&buf);
    char head;
    is.read(&head, 1);

    if (head == MSG_LINE)
    {
        int line_count = __getlineToInt(is);
        redisReply re;
        while(true)
        {
            head = 0;
            is.read(&head, 1);
            //如果读到的数据不全则继续读
            //todo 这里需要写一个python脚本来测试下,如果数据分成多次读取的话会怎么样
            if (head == 0)
            {
                boost::asio::async_read_until( _connection, buf, "\r\n", yield);
            }

            redisReply tmp = this->parseReply(head, buf, is, yield);
            re.push( tmp );

            if ( re.size() == line_count )
            {
                return re;
            }
        }
    }
    else
    {
        return this->parseReply(head, buf, is, yield);
    }

}

asioRedis::~asioRedis()
{

}

void asioRedis::async_connect(const std::string &ip, const int port, const boost::asio::yield_context &yield)
{
    boost::asio::ip::tcp::endpoint ep( boost::asio::ip::address::from_string(ip), port );
    _connection.async_connect( ep, yield );
}

asioRedis::asioRedis(boost::asio::io_service &ios)
:_connection(ios),
 _strand(ios)
{

}
redisReply asioRedis::parseReply(const char head,
                                 boost::asio::streambuf &buf,
                                 std::istream &is,
                                 const boost::asio::yield_context &yield)
{
    redisReply re;

    switch(head)
    {
        case OK:
        {
            re.type = redisReply::REPLY_OK;
            break;
        }

        case ERROR:
        {
            std::string s = _getline(is);
            re.setErrorMessage(s);
            break;
        }


        case NEXT_LINE:
        {
            int next_line_size = __getlineToInt(is);
            next_line_size -= boost::asio::buffer_size(buf.data())-2;

            if( next_line_size > 0)
            {
                boost::asio::async_read( _connection, buf, boost::asio::transfer_exactly(next_line_size), yield);
            }

            std::string s = _getline(is);
            re.setStr(s);
            break;
        }

        case INT_VAL:
        {
            long long val;
            is >> val;
            re.setInteger(val);
            buf.consume(2);
            break;
        }

        default:
            break;
    }
    return re;
}
