#include "pch.h"
#include "tcpio.h"

#ifdef __linux__
#	define ADDRESS_LEN_T unsigned int
#	define get_last_error() errno
#	define cleanup()
#	define close_socket(socket) ::close(socket)
#else
#	define ADDRESS_LEN_T int
#	define get_last_error() WSAGetLastError()
#	define cleanup() WSACleanup()
#	define close_socket(socket) ::closesocket(socket)
#endif

int tcp_server_t::wait_for_client(const char* your_address, const int your_port, const char* your_client_address, const int your_client_port)
{
	SOCKET server_socket;

#ifndef __linux__
	//Initialise winsock
	if (WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0)
	{
		int error_code = get_last_error();
		return error_code;
	}
#endif

	//Create a socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();
		cleanup();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	you.sin_family = AF_INET;
	you.sin_addr.s_addr = inet_addr(your_address);
	you.sin_port = htons(your_port);

	//Bind
	if (bind(server_socket, (sockaddr*)&you, sizeof(you)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close_socket(server_socket);
		close();

		return error_code;
	}

	//Listen
	if (listen(server_socket, 1) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close_socket(server_socket);
		close();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	your_client.sin_family = AF_INET;
	your_client.sin_addr.s_addr = inet_addr(your_client_address);
	your_client.sin_port = htons(your_client_port);

	//Accept
	ADDRESS_LEN_T sizeof_your_client = sizeof(your_client);
	if ((the_socket = accept(server_socket, (sockaddr*)&your_client, &sizeof_your_client)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();
		close_socket(server_socket);
		close();

		return error_code;
	}

	close_socket(server_socket);
	return 0;
}

int tcp_server_t::close()
{
	close_socket(the_socket);
	cleanup();

	return 0;
}

int tcp_server_t::send(const char* packet, const int size)
{
	const char* offset{ packet };
	int to_send{ size };

	do
	{
		const int&& ret = ::send(the_socket, offset, to_send, 0);
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

int tcp_server_t::recv(char* packet, const int size)
{
	char* offset{ packet };
	int to_receive{ size };

	do
	{
		const int&& ret = ::recv(the_socket, offset, to_receive, 0);
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

int tcp_server_t::recv_any(char* packet, const int capacity, int* recvd_size)
{
	const int&& ret = ::recv(the_socket, packet, capacity, 0);
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

tcp_server_t::~tcp_server_t()
{
	close();
}

int tcp_client_t::connect_to_server(const char* your_address, const int your_port, const char* your_server_address, const int your_server_port)
{
#ifndef __linux__
	//Initialise winsock
	if (WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0)
	{
		int error_code = get_last_error();
		return error_code;
	}
#endif

	//Create a socket
	if ((the_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();
		cleanup();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	you.sin_family = AF_INET;
	you.sin_addr.s_addr = inet_addr(your_address);
	you.sin_port = htons(your_port);

	//Bind
	if (bind(the_socket, (sockaddr*)&you, sizeof(you)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	your_server.sin_family = AF_INET;
	your_server.sin_addr.s_addr = inet_addr(your_server_address);
	your_server.sin_port = htons(your_server_port);

	//Listen
	if (connect(the_socket, (sockaddr*)&your_server, sizeof(your_server)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	return 0;
}

int tcp_client_t::close()
{
	close_socket(the_socket);
	cleanup();

	return 0;
}

int tcp_client_t::send(const char* packet, const int size)
{
	const char* offset{ packet };
	int to_send{ size };

	do
	{
		const int&& ret = ::send(the_socket, offset, to_send, 0);
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

int tcp_client_t::recv(char* packet, const int size)
{
	char* offset{ packet };
	int to_receive{ size };

	do
	{
		const int&& ret = ::recv(the_socket, offset, to_receive, 0);
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

int tcp_client_t::recv_any(char* packet, const int capacity, int* recvd_size)
{
	const int&& ret = ::recv(the_socket, packet, capacity, 0);
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

tcp_client_t::~tcp_client_t()
{
	close();
}
