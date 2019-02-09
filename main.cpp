#include "pch.h"

#include "io/sockio.h"
#include "sync/resettable_event.h"

#define PACKET_LEN (1500)

manual_reset_event ready{ false }, finished{ false };
uint16_t rx_port{ 0 };
ip_protocol_t protocol{ ip_protocol_t::tcp };

tcp_server_t server;
socket_t transmitter, receiver;

void rx();
void tx(uint16_t port);

int main()
{
	INIT();

	do
	{
		int t;

		printf("Protocol(0: TCP, 1:UDP)? ");
		scanf("%d", &t);
		if (t == 0)
		{
			protocol = ip_protocol_t::tcp;
			break;
		}
		else if (t == 1)
		{
			protocol = ip_protocol_t::udp;
			break;
		}
	} while (true);

	std::thread rx_thread{ rx };
	ready.wait();
	std::thread tx_thread{ tx, rx_port };

	printf("press enter to stop... \n");
	FLUSH_OUT();
	GET_CHAR();

	finished.set();

	if (protocol == ip_protocol_t::tcp)
	{
		server.close();
	}
	transmitter.close();
	receiver.close();

	tx_thread.join();
	rx_thread.join();

	FINISH(0);
}

void rx()
{
	char packet[PACKET_LEN];
	int ret;

	if (protocol == ip_protocol_t::tcp)
	{
		ret = server.create("127.0.0.1");
		if (ret != 0)
		{
			printf("create server failed. error code: %d \n", ret);
			return;
		}

		rx_port = server.port();
		ready.set();

		ret = server.listen(receiver);
		if (ret != 0)
		{
			if (!finished.is_set())
			{
				printf("listen failed. error code: %d \n", ret);
			}
			return;
		}
	}
	else
	{
		ret = receiver.create(ip_protocol_t::udp, "127.0.0.1");
		if (ret != 0)
		{
			printf("create server failed. error code: %d \n", ret);
			return;
		}

		rx_port = receiver.mine_port();
		ready.set();
	}
	

	while (!finished.is_set())
	{
		ret = receiver.recv(packet, PACKET_LEN);
		if (ret != 0)
		{
			if (!finished.is_set())
			{
				printf("recv failed. error code: %d \n", ret);
			}
			break;
		}

		printf("recv %d OK. \n", *(int*)&packet[0]);
	}

	printf("rx closed. \n");
}

void tx(uint16_t port)
{
	char packet[PACKET_LEN];
	int cnt;
	int ret;

	ret = transmitter.create(protocol);
	if (ret != 0)
	{
		printf("create failed. error code: %d \n", ret);
		return;
	}

	ret = transmitter.connect("127.0.0.1", port);
	if (ret != 0)
	{
		printf("connect failed. error code: %d \n", ret);
		return;
	}

	cnt = 0;

	while (!finished.wait_for(std::chrono::milliseconds(1500)))
	{
		printf("\n");

		*(int*)&packet[0] = cnt;
		ret = transmitter.send(packet, PACKET_LEN);
		if (ret != 0)
		{
			printf("send failed. error code: %d \n", ret);
			break;
		}

		printf("send %d OK. \n", cnt);
		++cnt;
	}

	printf("tx closed. \n");
}
