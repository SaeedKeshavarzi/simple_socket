#include <cassert>
#include <string>

#ifdef __linux__

#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <unistd.h>
#	include <errno.h>

typedef int SOCKET;

#	define INVALID_SOCKET   (SOCKET)(~0)
#	define SOCKET_ERROR     (-1)
#	define ADDRESS_LEN_T unsigned int
#	define get_last_error() errno
#	define close_socket(socket_id) ::close(socket_id)

#else

#	define _WINSOCK_DEPRECATED_NO_WARNINGS

#	include <WinSock2.h>
#	include <iphlpapi.h>
#	pragma comment(lib, "IPHLPAPI.lib")
#	pragma comment(lib, "ws2_32.lib")

#	define ADDRESS_LEN_T int
#	define get_last_error() WSAGetLastError()
#	define close_socket(socket_id) ::closesocket(socket_id)

struct winsock_initializer_t
{
	WSADATA winsock_data;

public:
	winsock_initializer_t()
	{
		int ret = WSAStartup(MAKEWORD(2, 2), &winsock_data);
		if (ret != 0)
		{
			int error_code = get_last_error();
			assert(ret != 0);
		}
	}

	~winsock_initializer_t()
	{
		WSACleanup();
	}
} static instance;

#endif

#include "sockio.h"

int socket_t::create(const ip_protocol_t protocol, const std::string & ip, const uint16_t port)
{
	//Create a socket
	if ((socket_id = ::socket(AF_INET, protocol == ip_protocol_t::tcp ? SOCK_STREAM : SOCK_DGRAM, (int)protocol)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = (ip.empty() ? INADDR_ANY : inet_addr(ip.c_str()));
	address.sin_port = htons(port);

	//Bind
	if (bind(socket_id, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	return 0;
}

int socket_t::connect(const std::string & pair_ip, const uint16_t pair_port)
{
	assert(!pair_ip.empty());
	assert(pair_port > 0);

	//Prepare the sockaddr_in structure
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(pair_ip.c_str());
	address.sin_port = htons(pair_port);

	//connect
	if (::connect(socket_id, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	return 0;
}

int socket_t::send(const char * packet, const int size)
{
	const char * offset{ packet };
	int to_send{ size };

	do
	{
		const int && ret = ::send(socket_id, offset, to_send, 0);
		if (ret == SOCKET_ERROR)
		{
			int error_code = get_last_error();
			close();

			return error_code;
		}

		to_send -= ret;
		offset += ret;
	} while (to_send > 0);

	return 0;
}

int socket_t::send_to(const std::string & pair_ip, const uint16_t pair_port, const char * packet, const int size)
{
	assert(!pair_ip.empty());
	assert(pair_port > 0);

	//Prepare the sockaddr_in structure
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(pair_ip.c_str());
	address.sin_port = htons(pair_port);

	const char * offset{ packet };
	int to_send{ size };

	do
	{
		const int && ret = ::sendto(socket_id, offset, to_send, 0, (sockaddr*)&address, sizeof(address));
		if (ret == SOCKET_ERROR)
		{
			int error_code = get_last_error();
			close();

			return error_code;
		}

		to_send -= ret;
		offset += ret;
	} while (to_send > 0);

	return 0;
}

int socket_t::recv(char * packet, const int size)
{
	char * offset{ packet };
	int to_receive{ size };

	do
	{
		const int && ret = ::recv(socket_id, offset, to_receive, 0);
		if (ret == 0) // connection closed
		{
			close();

			return -1;
		}

		if (ret == SOCKET_ERROR)
		{
			int error_code = get_last_error();
			close();

			return error_code;
		}

		to_receive -= ret;
		offset += ret;
	} while (to_receive > 0);

	return 0;
}

int socket_t::recv_any(char * packet, const int capacity, int & recvd_size)
{
	const int && ret = ::recv(socket_id, packet, capacity, 0);
	if (ret == 0) // connection closed
	{
		close();

		return -1;
	}

	if (ret == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	recvd_size = ret;

	return 0;
}

int socket_t::recv_from(char * packet, const int size, std::string & pair_ip, uint16_t & pair_port)
{
	char * offset{ packet };
	int to_receive{ size };

	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);

	do
	{
		const int && ret = ::recvfrom(socket_id, offset, to_receive, 0, (sockaddr*)&address, &address_len);
		if (ret == 0) // connection closed
		{
			close();

			return -1;
		}

		if (ret == SOCKET_ERROR)
		{
			int error_code = get_last_error();
			close();

			return error_code;
		}

		to_receive -= ret;
		offset += ret;
	} while (to_receive > 0);

	pair_ip = std::string(inet_ntoa(address.sin_addr));
	pair_port = htons(address.sin_port);

	return 0;
}

int socket_t::recv_any_from(char * packet, const int capacity, int & recvd_size, std::string & pair_ip, uint16_t & pair_port)
{
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	const int && ret = ::recvfrom(socket_id, packet, capacity, 0, (sockaddr*)&address, &address_len);
	if (ret == 0) // connection closed
	{
		close();

		return -1;
	}

	if (ret == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	recvd_size = ret;
	pair_ip = std::string(inet_ntoa(address.sin_addr));
	pair_port = htons(address.sin_port);

	return 0;
}

std::string socket_t::mine_ip() const
{
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	getsockname(socket_id, (sockaddr*)&address, &address_len);

	return std::string(inet_ntoa(address.sin_addr));
}

uint16_t socket_t::mine_port() const
{
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	getsockname(socket_id, (sockaddr*)&address, &address_len);

	return htons(address.sin_port);
}

std::string socket_t::pair_ip() const
{
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	getpeername(socket_id, (sockaddr*)&address, &address_len);

	return std::string(inet_ntoa(address.sin_addr));
}

uint16_t socket_t::pair_port() const
{
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	getpeername(socket_id, (sockaddr*)&address, &address_len);

	return htons(address.sin_port);
}

int socket_t::close()
{
	close_socket(socket_id);

	return 0;
}

socket_t::~socket_t()
{
	close();
}

int tcp_server_t::create(const std::string & ip, const uint16_t port)
{
	//Create a socket
	if ((server_id = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = (ip.empty() ? INADDR_ANY : inet_addr(ip.c_str()));
	address.sin_port = htons(port);

	//Bind
	if (bind(server_id, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	return 0;
}

int tcp_server_t::listen(socket_t & client_socket)
{
	//Listen
	if (::listen(server_id, 1) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	//Accept
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	if ((client_socket.socket_id = accept(server_id, (sockaddr*)&address, &address_len)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	return 0;
}

std::string tcp_server_t::ip() const
{
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	getsockname(server_id, (sockaddr*)&address, &address_len);

	return std::string(inet_ntoa(address.sin_addr));
}

uint16_t tcp_server_t::port() const
{
	sockaddr_in address;
	ADDRESS_LEN_T address_len = sizeof(address);
	getsockname(server_id, (sockaddr*)&address, &address_len);

	return htons(address.sin_port);
}

int tcp_server_t::close()
{
	close_socket(server_id);

	return 0;
}

tcp_server_t::~tcp_server_t()
{
	close();
}
