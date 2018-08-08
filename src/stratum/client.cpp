#include <boost/bind.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fingera/stratum/client.hpp>
#include <iostream>

namespace fingera {
namespace stratum {

namespace pt = boost::property_tree;

client::client(boost::asio::io_service &io_service, boost::asio::ip::tcp::resolver::results_type endpoint,
    const std::string &user, const std::string &pass)
    : _io_service(io_service), _socket(io_service), _endpoint(endpoint), _user(user), _pass(pass),
    _ping_timer(io_service), _timer(io_service), _sequence(0)
{}
client::~client() {
}

void client::_delay_connect() {
    if (_is_reconnecting) return;
    _is_reconnecting = true;
    boost::system::error_code ec;
    _socket.close(ec);
    std::cout << "client::_delay_connect " << ec << std::endl;
    _timer.expires_after(boost::asio::chrono::seconds(5));
    _timer.async_wait(std::bind(&client::_do_connect, this));
}

void client::_do_connect() {
    _is_reconnecting = false;
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
    // std::cout << "client::_do_read" << std::endl;
    boost::asio::async_read_until(_socket, _response, "\n", 
        boost::bind(&client::_on_message, this,  boost::asio::placeholders::error));
}


void client::_start_keep_alive() {
    // TODO: Keep Alive
}

void client::_do_ping() {
    std::cout << "client::_do_ping" << std::endl;
    // id _sequence
    // jsonrpc 2.0
    // method keepalived
    // params {
    //     id _rpc_id
    // }
    std::stringstream sstream;
    sstream
        << "{"
        <<     "\"id\":" << _sequence << ","
        <<     "\"jsonrpc\":2.0,"
        <<     "\"method\":\"keepalived\","
        <<     "\"params\":{"
        <<         "\"id\":\"" << _rpc_id << "\"" // 6ac05719-dd1a-403a-a7c9-58eab5a66fad
        <<     "}"
        << "}\n";

    _write(sstream.str());
}

static bool is_critical_error(boost::optional<std::string> &message) {
    if (!message) {
        return false;
    }
    if (strncasecmp(message->c_str(), "Unauthenticated", 15) == 0) {
        return true;
    }
    if (strncasecmp(message->c_str(), "your IP is banned", 17) == 0) {
        return true;
    }
    if (strncasecmp(message->c_str(), "IP Address currently banned", 27) == 0) {
        return true;
    }
    return false;
}

void client::_on_message(const boost::system::error_code& err) {
    // std::cout << "client::_on_message" << std::endl;
    if (err) {
        std::cerr << err << std::endl;
        _delay_connect();
        return;
    }
    std::stringstream sout;
    sout << &_response;
    // std::cout << sout.str() << std::endl;

    pt::ptree tree;
    pt::read_json(sout, tree);

    auto id = tree.get_optional<int64_t>("id");
    auto jsonrpc = tree.get<std::string>("jsonrpc");
    if (jsonrpc != "2.0") {
        std::cerr << "bad json rpc version: " << jsonrpc << std::endl;
    }
    if (id) {
        // Response
        auto result = tree.get_child_optional("result");
        auto error = tree.get_child_optional("error");
        if (error && !error->empty()) {
            std::cout << "has error" << std::endl;
            auto message = error->get_optional<std::string>("message");
            // TODO: ResultAccepted Error
            if (is_critical_error(message)) {
                _delay_connect();
                return;
            }
        } else if (result) {
            if (*id == 1) {
                // Login
                auto result_status = result->get_optional<std::string>("status");
                if (!result_status) {
                    std::cerr << "bad login status " << sout.str() << std::endl;
                    _delay_connect();
                    return;
                }
                if (*result_status == "OK") {
                    auto result_id = result->get_optional<std::string>("id");
                    if (!result_id) {
                        std::cerr << "bad login id " << sout.str() << std::endl;
                        _delay_connect();
                        return;
                    }
                    _rpc_id = *result_id;
                    std::cout << "Login sucess " << _user << std::endl;
                    auto job = result->get_child_optional("job");
                    if (job) {
                        _parse_job(*job);
                    }
                }
            } else {
                // TODO: ResultAccepted Success
            }
        }
    } else {
        auto method = tree.get_optional<std::string>("method");
        if (!method) {
            std::cerr << "null method" << std::endl;
        } else if (*method != "job") {
            std::cerr << "unknow method: " << *method << std::endl;
        } else {
            auto params = tree.get_child_optional("params");
            if (!params) {
                std::cerr << "null params" << std::endl;
            } else {
                _parse_job(*params);
            }
        }
    }

    _do_read();
}

void client::_parse_job(const boost::property_tree::ptree &tree) {
    auto job_id = tree.get_optional<std::string>("job_id");
    auto blob = tree.get_optional<std::string>("blob");
    auto target = tree.get_optional<std::string>("target");
    auto id = tree.get_optional<std::string>("id");

    if (!job_id || !blob || !target || !id) {
        std::cerr << "client::_parse_job Invalid Packet" << std::endl;
        return;
    }

    _on_job(*id, *job_id, *blob, *target);
}

void client::_on_job(const std::string &id, const std::string &job_id, const std::string &blob, const std::string &target) {
    std::cout << "new job: " << id << " " << job_id << " " << blob << " " << target << std::endl;
    // 
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
