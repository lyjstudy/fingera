#pragma once

#include <deque>
#include <boost/asio.hpp>

namespace fingera {
namespace stratum {

class client {
public:
    client(boost::asio::io_service &io_service, boost::asio::ip::tcp::resolver::results_type endpoint,
        const std::string &user, const std::string &pass);
    virtual ~client();

    virtual void login();
protected:
    boost::asio::ip::tcp::resolver::results_type _endpoint;
    boost::asio::io_service &_io_service;
    boost::asio::ip::tcp::socket _socket;
    std::string _user;
    std::string _pass;
    boost::asio::steady_timer _timer;
    std::deque<std::string> _write_queue;
    boost::asio::streambuf _response;
    int64_t _sequence;

    void _do_connect();
    void _do_login();
    void _delay_connect();
    void _do_write();
    void _do_read();
    void _on_message(const boost::system::error_code& err);
    void _write(const std::string &buffer);

    void _start_keep_alive();
    void _do_ping();
};

} // namespace stratum
} // namespace fingera
