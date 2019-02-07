#ifndef _SOCKIO_H_
#define _SOCKIO_H_

#include <string>
#include <stdint.h>

enum class ip_protocol_t : int
{
	tcp = IPPROTO_TCP,
	udp = IPPROTO_UDP
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
	SOCKET socket_id;
	friend class tcp_server_t;
};

class tcp_server_t
{
public:
	int create(const std::string & ip = "", const uint16_t port = 0);
	int listen(socket_t & client_socket);

	std::string ip() const;
	uint16_t port() const;

	int close();
	~tcp_server_t();

private:
	SOCKET server_id;
};

#endif // !_SOCKIO_H_
