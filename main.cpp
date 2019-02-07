#include "pch.h"

#include "io/sockio.h"
#include "sync/resettable_event.h"

#define PACKET_LEN (1500)

manual_reset_event ready{ false };
uint16_t receiver_port{ 0 };
ip_protocol_t protocol{ ip_protocol_t::tcp };

bool keep_recv{ true };
bool keep_send{ true };

void rx_tcp_server();
void rx_udp_receiver();
void tx_transmitter(uint16_t port);

int main()
{
	INIT();

	do
	{
		int t;

		printf("Protocol(0: TCP, 1:UDP)? ");
		scanf("%d", &t);
		if ((t == 0) || (t == 1))
		{
			protocol = (t == 0 ? ip_protocol_t::tcp : ip_protocol_t::udp);
			break;
		}
	} while (true);

	printf("press enter to stop... \n");

	std::thread server_thread;
	if (protocol == ip_protocol_t::tcp)
	{
		server_thread = std::thread{ rx_tcp_server };
	}
	else // UDP
	{
		server_thread = std::thread{ rx_udp_receiver };
	}

	ready.wait();
	std::thread client_thread{ tx_transmitter, receiver_port };

	GET_CHAR();
	keep_recv = false;

	server_thread.join();
	client_thread.join();

	FINISH(0);
}

void rx_tcp_server()
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

	receiver_port = tcp_server.port();
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

		printf("recv %d OK. \n", *(int*)&packet[0]);
	}

	keep_send = false;

	socket.close();
	tcp_server.close();
}

void rx_udp_receiver()
{
	socket_t socket;
	char packet[PACKET_LEN];
	int ret;

	ret = socket.create(ip_protocol_t::udp, "127.0.0.1");
	if (ret != 0)
	{
		printf("create server failed. error code: %d \n", ret);
		return;
	}

	receiver_port = socket.mine_port();
	ready.set();

	while (keep_recv)
	{
		ret = socket.recv(packet, PACKET_LEN);
		if (ret != 0)
		{
			printf("recv failed. error code: %d \n", ret);
			break;
		}

		printf("recv %d OK. \n", *(int*)&packet[0]);
	}

	keep_send = false;

	socket.close();
}

void tx_transmitter(uint16_t port)
{
	socket_t transmitter;
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

	while (keep_send)
	{
		*(int*)&packet[0] = cnt;
		ret = transmitter.send(packet, PACKET_LEN);
		if (ret != 0)
		{
			printf("send failed. error code: %d \n", ret);
			break;
		}

		printf("send %d OK. \n", cnt);
		++cnt;

		std::this_thread::sleep_for(std::chrono::milliseconds(450));
	}

	transmitter.close();
}
