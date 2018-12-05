#include "pch.h"
#include "sockio.h"

#ifdef __linux__
#	define ADDRESS_LEN_T unsigned int
#	define get_last_error() errno
#	define close_socket(socket) ::close(socket)
#else
#	define ADDRESS_LEN_T int
#	define get_last_error() WSAGetLastError()
#	define close_socket(socket) ::closesocket(socket)

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

int socket_t::connect(const std::string & mine_ip, const int mine_port, const std::string & pair_ip, const int pair_port, const ip_protocol_t protocol)
{
	assert(!pair_ip.empty());

	//Create a socket
	if ((socket = ::socket(AF_INET, protocol == ip_protocol_t::tcp ? SOCK_STREAM : SOCK_DGRAM, (int)protocol)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	mine_address.sin_family = AF_INET;
	mine_address.sin_addr.s_addr = (mine_ip.empty() ? INADDR_ANY : inet_addr(mine_ip.c_str()));
	mine_address.sin_port = htons(mine_port);

	//Bind
	if (bind(socket, (sockaddr*)&mine_address, sizeof(mine_address)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	pair_address.sin_family = AF_INET;
	pair_address.sin_addr.s_addr = inet_addr(pair_ip.c_str());
	pair_address.sin_port = htons(pair_port);

	//connect
	if (::connect(socket, (sockaddr*)&pair_address, sizeof(pair_address)) == SOCKET_ERROR)
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
		const int && ret = ::send(socket, offset, to_send, 0);
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
		const int && ret = ::recv(socket, offset, to_receive, 0);
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

int socket_t::recv_any(char * packet, const int capacity, int * recvd_size)
{
	const int && ret = ::recv(socket, packet, capacity, 0);
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

	if (recvd_size != nullptr)
	{
		*recvd_size = ret;
	}

	return 0;
}

int socket_t::close()
{
	close_socket(socket);

	return 0;
}

socket_t::~socket_t()
{
	close();
}

int tcp_server_t::create(const std::string & ip, const int port)
{
	assert(port >= 0);

	//Create a socket
	if ((socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	ip_address.sin_family = AF_INET;
	ip_address.sin_addr.s_addr = (ip.empty() ? INADDR_ANY : inet_addr(ip.c_str()));
	ip_address.sin_port = htons(port);

	//Bind
	if (bind(socket, (sockaddr*)&ip_address, sizeof(ip_address)) == SOCKET_ERROR)
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
	if (::listen(socket, 1) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	//Accept
	ADDRESS_LEN_T sizeof_client_address = sizeof(client_socket.mine_address);
	if ((client_socket.socket = accept(socket, (sockaddr*)&client_socket.mine_address, &sizeof_client_address)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	client_socket.pair_address = ip_address;

	return 0;
}

int tcp_server_t::close()
{
	close_socket(socket);

	return 0;
}

tcp_server_t::~tcp_server_t()
{
	close();
}
