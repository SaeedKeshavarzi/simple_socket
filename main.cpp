#include "pch.h"

#include "io/tcpio.h"

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
	char packet[PACKET_LEN];
	int ret;

	ret = tcp_server.wait_for_client("192.168.1.102", 7121, "192.168.1.102", 7171);
	if (ret != 0)
	{
		printf("wait_for_client failed. error code: %d \n", ret);
		return;
	}

	while (keep_recv)
	{
		ret = tcp_server.recv(packet, PACKET_LEN);
		if (ret != 0)
		{
			printf("recv failed. error code: %d \n", ret);
			break;
		}

		printf("recv OK. \n");
	}

	keep_send = false;
}

void tx_client()
{
	tcp_client_t tcp_client;
	char packet[PACKET_LEN];
	int ret;

	ret = tcp_client.connect_to_server("192.168.1.102", 7171, "192.168.1.102", 7121);
	if (ret != 0)
	{
		printf("connect_to_server failed. error code: %d \n", ret);
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
}
