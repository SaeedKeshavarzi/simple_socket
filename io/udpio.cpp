#include "pch.h"
#include "udpio.h"

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

int udpio_t::open(const char* your_address, const int your_port, const char* your_friends_address, const int your_friends_port)
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
	if ((the_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		int error_code = get_last_error();
		cleanup();

		return error_code;
	}

	//Prepare the sockaddr_in structure
	you.sin_family = AF_INET;
	you.sin_addr.s_addr = inet_addr(your_address);
	you.sin_port = htons(your_port);

	//Prepare the sockaddr_in structure
	your_friend.sin_family = AF_INET;
	your_friend.sin_addr.s_addr = inet_addr(your_friends_address);
	your_friend.sin_port = htons(your_friends_port);

	//Bind
	if (bind(the_socket, (sockaddr*)&you, sizeof(you)) == SOCKET_ERROR)
	{
		int error_code = get_last_error();
		close();

		return error_code;
	}

	return 0;
}

int udpio_t::close()
{
	close_socket(the_socket);
	cleanup();

	return 0;
}

int udpio_t::send(const char* packet, const int size)
{
	const char* offset{ packet };
	int to_send{ size };

	do
	{
		const int&& ret = sendto(the_socket, offset, to_send, 0, (sockaddr*)&your_friend, sizeof(your_friend));
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

int udpio_t::recv(char* packet, const int size)
{
	ADDRESS_LEN_T sizeof_your_friend = sizeof(your_friend);
	char* offset{ packet };
	int to_receive{ size };

	do
	{
		const int&& ret = recvfrom(the_socket, offset, to_receive, 0, (sockaddr*)&your_friend, &sizeof_your_friend);
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

int udpio_t::recv_any(char* packet, const int capacity, int* recvd_size)
{
	ADDRESS_LEN_T sizeof_your_friend = sizeof(your_friend);
	const int&& ret = recvfrom(the_socket, packet, capacity, 0, (sockaddr*)&your_friend, &sizeof_your_friend);
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

udpio_t::~udpio_t()
{
	close();
}
