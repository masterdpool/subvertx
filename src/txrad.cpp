#include <boost/lexical_cast.hpp>

#include <bitcoin/bitcoin.hpp>
using namespace libbitcoin;
using std::placeholders::_1;
using std::placeholders::_2;

class radar;
typedef std::shared_ptr<radar> radar_ptr;

void null_handler(const std::error_code& ec)
{
}

class radar
 : public std::enable_shared_from_this<radar>
{
public:
    static radar_ptr create();

    radar(const radar&) = delete;
    void operator=(const radar&) = delete;

    void start();
private:
    radar();
    void initialize();

    void check_exit(const std::error_code& ec);

    void initial_handshake(const std::error_code& ec, channel_ptr node);
    void request_addresses();
    void receive_addr(const std::error_code& ec,
        const address_type& packet);
    void monitor(const std::error_code& ec, channel_ptr node);
    void check_invs(const std::error_code& ec, const inventory_type& inv,
        channel_ptr node);

    async_service service_;
    network network_;
    handshake handshake_;
    channel_ptr feeder_;
    size_t counter_;
    std::map<hash_digest, size_t> seen_txs_;
};

radar::radar()
  : network_(service_), handshake_(service_), feeder_(nullptr)
{
}
radar_ptr radar::create()
{
    radar_ptr result(new radar);
    result->initialize();
    return result;
}
void radar::initialize()
{
    service_.spawn();
    counter_ = 0;
}

void radar::start()
{
    connect(handshake_, network_, "localhost", 8333,
        std::bind(&radar::initial_handshake, shared_from_this(), _1, _2));
}

void radar::check_exit(const std::error_code& ec)
{
    if (ec)
    {
        log_error() << "txrad: " << ec.message();
        exit(1);
    }
}

void radar::initial_handshake(const std::error_code& ec, channel_ptr node)
{
    check_exit(ec);
    feeder_ = node;
    request_addresses();
}

void radar::request_addresses()
{
    feeder_->subscribe_address(
        std::bind(&radar::receive_addr, shared_from_this(), _1, _2));
    feeder_->send(get_address_type(), null_handler);
}

std::string char_repr(uint8_t c)
{
    return boost::lexical_cast<std::string>(static_cast<size_t>(c));
}
void radar::receive_addr(const std::error_code& ec,
    const address_type& packet)
{
    check_exit(ec);
    for (const network_address_type& netaddr: packet.addresses)
    {
        // Only support ipv4 cos I'm lazy :(
        if (netaddr.ip[10] != 0xff && netaddr.ip[11] != 0xff)
            continue;
        std::string ip_repr =
            char_repr(netaddr.ip[12]) + "." +
            char_repr(netaddr.ip[13]) + "." +
            char_repr(netaddr.ip[14]) + "." +
            char_repr(netaddr.ip[15]);
        log_debug() << "Connecting to: " << ip_repr;
        connect(handshake_, network_, ip_repr, 8333,
            std::bind(&radar::monitor, shared_from_this(), _1, _2));
    }
}

void radar::monitor(const std::error_code& ec, channel_ptr node)
{
    if (++counter_ < 100)
        request_addresses();
    if (ec)
    {
        log_warning() << ec.message();
        return;
    }
    node->subscribe_inventory(
        std::bind(&radar::check_invs, shared_from_this(), _1, _2, node));
}

void radar::check_invs(const std::error_code& ec,
    const inventory_type& inv, channel_ptr node)
{
    check_exit(ec);
    for (const inventory_vector_type& ivec: inv.inventories)
    {
        // Only interested in txs
        if (ivec.type != inventory_type_id::transaction)
            continue;
        log_debug() << "Found " << pretty_hex(ivec.hash) << "!";
    }
    node->subscribe_inventory(
        std::bind(&radar::check_invs, shared_from_this(), _1, _2, node));
}

int main()
{
    radar_ptr r = radar::create();
    r->start();
    // Sleep forever
    while (true)
        sleep(1000);
    return 0;
}

