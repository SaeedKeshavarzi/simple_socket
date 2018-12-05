#include "pch.h"

#include "io/sockio.h"

#define PACKET_LEN (1500)

bool keep_recv{ true };
bool keep_send{ true };
void rx_server();
void tx_client();

int main()
{
	INIT();

	printf("press enter to stop... \n");

	std::thread server_thread{ rx_server };
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::thread client_thread{ tx_client };

	GET_CHAR();
	keep_recv = false;

	server_thread.join();
	client_thread.join();

	FINISH(0);
}

void rx_server()
{
	tcp_server_t tcp_server;
	socket_t socket;
	char packet[PACKET_LEN];
	int ret;

	ret = tcp_server.create("192.168.0.10", 7121);
	if (ret != 0)
	{
		printf("create server failed. error code: %d \n", ret);
		return;
	}

	ret = tcp_server.listen(socket);
	if (ret != 0)
	{
		printf("listen failed. error code: %d \n", ret);
		return;
	}

	while (keep_recv)
	{
		ret = socket.recv(packet, PACKET_LEN);
		if (ret != 0)
		{
			printf("recv failed. error code: %d \n", ret);
			break;
		}

		printf("recv OK. \n");
	}

	keep_send = false;

	socket.close();
	tcp_server.close();
}

void tx_client()
{
	socket_t tcp_client;
	char packet[PACKET_LEN];
	int ret;

	ret = tcp_client.connect("192.168.0.10", 7171, "192.168.0.10", 7121, ip_protocol_t::tcp);
	if (ret != 0)
	{
		printf("connect failed. error code: %d \n", ret);
		return;
	}

	while (keep_send)
	{
		ret = tcp_client.send(packet, PACKET_LEN);
		if (ret != 0)
		{
			printf("send failed. error code: %d \n", ret);
			break;
		}

		printf("send OK. \n");
		std::this_thread::sleep_for(std::chrono::milliseconds(450));
	}

	tcp_client.close();
}
