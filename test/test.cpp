


#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>

#include "asioRedis.h"

#define BOOST_TEST_MODULE example
#include <boost/test/included/unit_test.hpp>

void redisCoroutine(boost::asio::io_service &ios, const boost::asio::yield_context &yield)
{
    asioRedis redis(ios);
    try
    {
        redis.async_connect("127.0.0.1", 8081, yield);

        auto reply = redis.async_command("set h 12", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_OK);

        reply = redis.async_command("get h", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_STRING);
        BOOST_CHECK(reply.str() == "12");

        reply = redis.async_command("LPUSH l 12 13 14", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_INTEGER);
        int len = reply.integer();

        reply = redis.async_command("LLEN l", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_INTEGER);
        BOOST_CHECK(len == reply.integer());

        reply = redis.async_command("LRANGE l 0 1", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_ARRAY);

        reply = redis.async_command("LRANGE 0 10", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_ERROR);

        reply = redis.async_command("HSET hello file1 joker", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_INTEGER);

        reply = redis.async_command("HGET hello file1", yield);
        BOOST_CHECK(reply.type == redisReply::REPLY_STRING);
        BOOST_CHECK(reply.str() == "joker");
    }
    catch (std::exception &e)
    {
        std::cout<< e.what() << std::endl;
    }


}

BOOST_AUTO_TEST_SUITE(redis)

BOOST_AUTO_TEST_CASE(test_redis)
{
    boost::asio::io_service ios;
    auto fn = boost::bind( redisCoroutine, boost::ref(ios), _1 );
    boost::asio::spawn(ios, fn);
    ios.run();
}

BOOST_AUTO_TEST_SUITE_END()
