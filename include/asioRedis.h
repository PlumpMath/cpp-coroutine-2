//
// Created by linyilong3 on 16/9/30.
//

#ifndef ASIO_REDIS_ASREDIS_H
#define ASIO_REDIS_ASREDIS_H

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <string>
#include <iostream>


class redisReply
{
public:
    enum ReplyType
    {
        REPLY_STRING = 1,
        REPLY_ARRAY,
        REPLY_INTEGER,
        REPLY_ERROR,
        REPLY_NIL,
        REPLY_OK,
    };
    ReplyType type;

    long long integer() const
    {
        assert( type == REPLY_INTEGER );
        return _integer;
    }

    void setInteger( const long long integer)
    {
        _integer = integer;
        type = REPLY_INTEGER;
    }

    std::string str() const
    {
        assert( type == REPLY_STRING );
        return _str;
    }

    std::string errorMessage() const
    {
        assert( type == REPLY_ERROR );
        return _str;
    }

    void setErrorMessage( const std::string &msg )
    {
        type = REPLY_ERROR;
        _str = msg;
    }

    void setStr( const std::string &str )
    {
        _str = str;
        type = REPLY_STRING;
    }

    redisReply &index(const int i)
    {
        assert( type == REPLY_ARRAY );
        return _replys[i];
    }

    void push( redisReply &r )
    {
        type = REPLY_ARRAY;
        _replys.push_back(r);
    }

    size_t size()
    {
        assert( type == REPLY_ARRAY );
        return _replys.size();
    }

    redisReply()
    {
        _integer = 0;
        this->type = REPLY_NIL;
    }

    redisReply( redisReply &&other )
    {
        this->move(std::move(other));
    }

    redisReply( const redisReply &other )
    {
        this->copy(other);
    }

    redisReply &operator=( const redisReply &other)
    {
        this->copy(other);
        return *this;
    }

    redisReply &operator=( redisReply &&other )
    {
        this->move(std::move(other));
        return *this;
    }

protected:
    void move(redisReply &&other)
    {
        this->_integer = other._integer;
        this->_str = std::move(other._str);
        this->_replys = std::move(other._replys);
        this->type = other.type;

        other._integer = 0;
        other.type = REPLY_NIL;
    }

    void copy(const redisReply &other)
    {
        this->_integer = other._integer;
        this->_str = other._str;
        this->_replys = other._replys;
        this->type = other.type;
    }

    std::vector< redisReply > _replys;

    std::string _str;

    long long _integer;
};

class asioRedis
{
public:
    asioRedis(boost::asio::io_service &ios);

    ~asioRedis();

    void async_connect(const std::string &ip, const int port, const boost::asio::yield_context &yield);

    redisReply async_command(const std::string &command, const boost::asio::yield_context &yield);

    void disconnect();

protected:
    redisReply parseReply(const char head,
                          boost::asio::streambuf &buf,
                          std::istream &is,
                          const boost::asio::yield_context &yield);

    boost::asio::io_service::strand _strand;

    boost::asio::ip::tcp::socket _connection;
};


#endif //ASIO_REDIS_ASREDIS_H
