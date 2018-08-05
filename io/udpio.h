#ifndef _UDPIO_H_
#define _UDPIO_H_

//TODO: make winpcap udp version
class udpio_t
{
public:
	int open(const char* your_address, const int your_port, const char* your_friends_address, const int your_friends_port);
	int close();

	int send(const char* packet, const int size);
	int recv(char* packet, const int size);
	int recv_any(char* packet, const int capacity, int* recvd_size = nullptr);

	~udpio_t();

private:
	SOCKET the_socket;
	sockaddr_in you;
	sockaddr_in your_friend;
#ifndef __linux__
	WSADATA winsock_data;
#endif
};

#endif // !_UDPIO_H_
