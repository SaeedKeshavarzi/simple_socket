#include "pch.h"

#include "io/sockio.h"
#include "sync/resettable_event.h"

#define PACKET_LEN (1500)

manual_reset_event ready{ false };
uint16_t server_port{ 0 };

bool keep_recv{ true };
bool keep_send{ true };

void rx_server();
void tx_client(uint16_t port);

int main()
{
	INIT();

	printf("press enter to stop... \n");

	std::thread server_thread{ rx_server };

	ready.wait();
	std::thread client_thread{ tx_client, server_port };

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

	ret = tcp_server.create("127.0.0.1");
	if (ret != 0)
	{
		printf("create server failed. error code: %d \n", ret);
		return;
	}

	server_port = tcp_server.port();
	ready.set();

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

		printf("recv %d OK. \n\n", *(int*)&packet[0]);
	}

	keep_send = false;

	socket.close();
	tcp_server.close();
}

void tx_client(uint16_t port)
{
	socket_t tcp_client;
	char packet[PACKET_LEN];
	int cnt;
	int ret;

	ret = tcp_client.connect("127.0.0.1", 0, "127.0.0.1", port, ip_protocol_t::tcp);
	if (ret != 0)
	{
		printf("connect failed. error code: %d \n", ret);
		return;
	}

	cnt = 0;

	while (keep_send)
	{
		*(int*)&packet[0] = cnt;
		ret = tcp_client.send(packet, PACKET_LEN);
		if (ret != 0)
		{
			printf("send failed. error code: %d \n", ret);
			break;
		}

		printf("send %d OK. \n", cnt);
		++cnt;

		std::this_thread::sleep_for(std::chrono::milliseconds(450));
	}

	tcp_client.close();
}
