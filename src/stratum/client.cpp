#include <boost/bind.hpp>
#include <fingera/stratum/client.hpp>
#include <iostream>

namespace fingera {
namespace stratum {

client::client(boost::asio::io_service &io_service, boost::asio::ip::tcp::resolver::results_type endpoint,
    const std::string &user, const std::string &pass)
    : _io_service(io_service), _socket(io_service), _endpoint(endpoint), _user(user), _pass(pass),
    _timer(io_service), _sequence(0)
{}
client::~client() {
}

void client::_delay_connect() {
    _timer.expires_at(_timer.expiry() + boost::asio::chrono::seconds(5));
    _timer.async_wait(std::bind(&client::_do_connect, this));
}

void client::_do_connect() {
    std::cout << "client::_do_connect " << _user << " " << _pass << std::endl;
    boost::asio::async_connect(_socket, _endpoint,
        [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint e) {
            if (!ec) {
                _do_login();
            } else {
                std::cerr << "connect " << e << " error! reconnect 5 seconds" << std::endl;
                _delay_connect();
            }
        }
    );
}

void client::_do_login() {
    _sequence = 0;
    std::cout << "client::_do_login " << _user << " " << _pass << std::endl;
    // id 1
    // jsonrpc 2.0
    // method login
    // params {
    //     login username
    //     pass password
    //     agent fingera_monero_beta
    //     rigid null
    //     algo [
    //         "1",
    //     ]
    // }
    std::stringstream sstream;
    sstream
        << "{"
        <<     "\"id\":1,"
        <<     "\"jsonrpc\":2.0,"
        <<     "\"method\":\"login\","
        <<     "\"params\":{"
        <<         "\"login\":\"" << _user << "\","
        <<         "\"pass\":\"" << _pass << "\","
        <<         "\"agent\":\"fingera_monero_beta\","
        <<             "\"algo\":[\"1\"]"
        <<     "}"
        << "}\n";
    _write(sstream.str());
}

void client::_do_write() {
    std::cout << "client::_do_write: " << _write_queue.front() << std::endl;
    boost::asio::async_write(_socket, boost::asio::buffer(_write_queue.front()),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                _write_queue.pop_front();
                if (!_write_queue.empty()) {
                    _do_write();
                }
                _do_read();
            } else {
                _delay_connect();
            }
        }
    );
}

void client::_do_read() {
    std::cout << "client::_do_read" << std::endl;
    boost::asio::async_read_until(_socket, _response, "\n", 
        boost::bind(&client::_on_message, this,  boost::asio::placeholders::error));
}


void client::_start_keep_alive() {

}

void client::_do_ping() {
    std::cout << "client::_do_ping" << std::endl;
    // id _sequence
    // jsonrpc 2.0
    // method keepalived
    // params {
    //     id _rpc_id(parsed by login repy TODO TODO TODO)
    // }
    std::stringstream sstream;
    sstream
        << "{"
        <<     "\"id\":" << _sequence << ","
        <<     "\"jsonrpc\":2.0,"
        <<     "\"method\":\"keepalived\","
        <<     "\"params\":{"
        <<         "\"id\":\"" << _user << "\"" // 6ac05719-dd1a-403a-a7c9-58eab5a66fad
        <<     "}"
        << "}\n";

    _write(sstream.str());
}

void client::_on_message(const boost::system::error_code& err) {
    std::cout << "client::_on_message" << std::endl;
    if (err) {
        _delay_connect();
        return;
    }
    std::ostringstream sout;
    sout << &_response;
    std::cout << sout.str() << std::endl;
    _do_read();
}

void client::_write(const std::string &buffer) {
    _write_queue.push_back(buffer);
    if (_write_queue.size() == 1) {
        _do_write();
    }
    _sequence++;
}

void client::login() {
    _do_connect();
}

} // namespace stratum
} // namespace fingera
