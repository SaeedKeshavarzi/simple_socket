#ifndef _SOCKIO_H_
#define _SOCKIO_H_

#include <string>
#include <vector>
#include <stdint.h>

#ifdef __linux__

#	include <arpa/inet.h>

typedef int SOCKET;

#else

#	define _WINSOCK_DEPRECATED_NO_WARNINGS
#	include <WinSock2.h>

#endif

enum class ip_protocol_t : int
{
	tcp = IPPROTO_TCP,
	udp = IPPROTO_UDP
};

struct adapter_info_t
{
	std::string name;
	std::string mac_address;
	std::string ip_address;
	std::string ip_mask;
};

class socket_t
{
public:
	int create(const ip_protocol_t protocol, const std::string & ip = "", const uint16_t port = 0);
	int connect(const std::string & pair_ip, const uint16_t pair_port);

	int send(const char * packet, const int size);
	int send_to(const std::string & pair_ip, const uint16_t pair_port, const char * packet, const int size);

	int recv(char * packet, const int size);
	int recv_any(char * packet, const int capacity, int & recvd_size);
	int recv_from(char * packet, const int size, std::string & pair_ip, uint16_t & pair_port);
	int recv_any_from(char * packet, const int capacity, int & recvd_size, std::string & pair_ip, uint16_t & pair_port);

	std::string mine_ip() const;
	uint16_t mine_port() const;

	std::string pair_ip() const;
	uint16_t pair_port() const;

	int close();
	~socket_t();

private:
	SOCKET socket_id{ (SOCKET)0 };
	friend class tcp_server_t;
};

class tcp_server_t
{
public:
	int create(const std::string & ip = "", const uint16_t port = 0);
	int listen(socket_t & client_socket, const int backlog = 1);

	std::string ip() const;
	uint16_t port() const;

	int close();
	~tcp_server_t();

private:
	SOCKET server_id{ (SOCKET)0 };
};

class ip_helper
{
public:
	ip_helper() = delete;

	static std::vector<adapter_info_t> get_adapters_info();
};

#endif // !_SOCKIO_H_
