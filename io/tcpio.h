#ifndef _TCPIO_H_
#define _TCPIO_H_

class tcp_server_t
{
public:
	int wait_for_client(const char* your_address, const int your_port, const char* your_client_address, const int your_client_port);
	int close();

	int send(const char* packet, const int size);
	int recv(char* packet, const int size);
	int recv_any(char* packet, const int capacity, int* recvd_size = nullptr);

	~tcp_server_t();

private:
	SOCKET the_socket;
	sockaddr_in you;
	sockaddr_in your_client;
#ifndef __linux__
	WSADATA winsock_data;
#endif
};

class tcp_client_t
{
public:
	int connect_to_server(const char* your_address, const int your_port, const char* your_server_address, const int your_server_port);
	int close();

	int send(const char* packet, const int size);
	int recv(char* packet, const int size);
	int recv_any(char* packet, const int capacity, int* recvd_size = nullptr);

	~tcp_client_t();

private:
	SOCKET the_socket;
	sockaddr_in you;
	sockaddr_in your_server;
#ifndef __linux__
	WSADATA winsock_data;
#endif
};

#endif // !_TCPIO_H_
