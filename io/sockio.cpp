#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__

#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <ifaddrs.h>
#	include <netdb.h>
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

std::vector<adapter_info_t> ip_helper::get_adapters_info()
{
	std::vector<adapter_info_t> adapters_list;

	try
	{
#ifdef __linux__
		//struct ifaddrs * ifaddr, *ifa;
		//int family, s;
		//char host[NI_MAXHOST];

		//if (getifaddrs(&ifaddr) == -1)
		//{
		//	throw new std::exception("getifaddrs failed.");
		//}

		///* Walk through linked list, maintaining head pointer so we
		//can free list later */

		//for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
		//{
		//	if (ifa->ifa_addr == nullptr)
		//	{
		//		continue;
		//	}

		//	/* For an AF_INET* interface address, display the address */
		//	family = ifa->ifa_addr->sa_family;
		//	if (family == AF_INET || family == AF_INET6)
		//	{
		//		s = getnameinfo(ifa->ifa_addr,
		//			(family == AF_INET) ? sizeof(struct sockaddr_in) :
		//			sizeof(struct sockaddr_in6),
		//			host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		//		if (s != 0)
		//		{
		//			std::string exception_msg = "getnameinfo() failed: " + gai_strerror(s) + ".";
		//			throw new std::exception(exception_msg.c_str());
		//		}

		//		printf("\taddress: <%s>\n", host);
		//	}
		//}

		//freeifaddrs(ifaddr);
#else
		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;
		DWORD dwRetVal = 0;
		UINT i;

		/* variables used to print DHCP time info */
		struct tm newtime;
		char buffer[32];
		errno_t error;

		ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
		pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, sizeof(IP_ADAPTER_INFO));
		if (pAdapterInfo == NULL)
		{
			throw new std::exception("Error allocating memory needed to call GetAdaptersinfo.");
		}

		// Make an initial call to GetAdaptersInfo to get
		// the necessary size into the ulOutBufLen variable
		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			HeapFree(GetProcessHeap(), 0, pAdapterInfo);
			pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);
			if (pAdapterInfo == NULL)
			{
				throw new std::exception("Error allocating memory needed to call GetAdaptersinfo.");
			}
		}

		if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
		{
			char * buf = new char[100];

			for (pAdapter = pAdapterInfo; pAdapter != nullptr; pAdapter = pAdapter->Next)
			{
				adapter_info_t adapter_info;

				buf[0] = '\0';
				for (i = 0; i < pAdapter->AddressLength; ++i)
				{
					if (i == (pAdapter->AddressLength - 1))
					{
						sprintf(buf + strlen(buf), "%.2X", (int)pAdapter->Address[i]);
					}
					else
					{
						sprintf(buf + strlen(buf), "%.2X-", (int)pAdapter->Address[i]);
					}
				}

				adapter_info.mac_address = buf;
				adapter_info.name = pAdapter->Description;
				adapter_info.ip_address = pAdapter->IpAddressList.IpAddress.String;
				adapter_info.ip_mask = pAdapter->IpAddressList.IpMask.String;

				adapters_list.push_back(adapter_info);
			}

			delete[] buf;
		}
		else
		{
			std::string exception_msg = "GetAdaptersInfo failed with error: " + std::to_string(dwRetVal) + ".";
			throw new std::exception(exception_msg.c_str());
		}

		if (pAdapterInfo)
		{
			HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		}
#endif // __linux__
	}
	catch (const std::exception & ex)
	{

	}

	return adapters_list;
}

