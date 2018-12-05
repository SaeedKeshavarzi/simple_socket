#ifndef _SOCKIO_H_
#define _SOCKIO_H_

enum class ip_protocol_t : int
{
	tcp = IPPROTO_TCP,
	udp = IPPROTO_UDP
};

class socket_t
{
private:
	SOCKET socket;
	sockaddr_in mine_address;
	sockaddr_in pair_address;

	friend class tcp_server_t;

public:
	int connect(const std::string & mine_ip, const int mine_port, const std::string & pair_ip, const int pair_port, const ip_protocol_t protocol);

	int send(const char * packet, const int size);
	int recv(char * packet, const int size);
	int recv_any(char * packet, const int capacity, int * recvd_size = nullptr);

	int close();
	~socket_t();
};

class tcp_server_t
{
private:
	SOCKET socket;
	sockaddr_in ip_address;

public:
	int create(const std::string & ip, const int port);
	int listen(socket_t & client_socket);
	int close();

	~tcp_server_t();
};

#endif // !_SOCKIO_H_
