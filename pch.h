#ifndef _PCH_H_
#define _PCH_H_

#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif

#include <map>
#include <set>
#include <list>
#include <queue>
#include <stack>
#include <deque>
#include <ctime>
#include <mutex>
#include <bitset>
#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
#include <vector>
#include <thread>
#include <atomic>
#include <cctype>
#include <string>
#include <cstdlib>
#include <cassert>
#include <utility>
#include <complex>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <string.h>
#include <memory.h>
#include <iostream>
#include <algorithm>
#include <experimental/filesystem>

#ifdef __linux__

#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <unistd.h>
#	include <errno.h>

typedef int SOCKET;

#	define INVALID_SOCKET   (SOCKET)(~0)
#	define SOCKET_ERROR     (-1)

#else

#	define _WINSOCK_DEPRECATED_NO_WARNINGS

#	include <WinSock2.h>
#	include <iphlpapi.h>
#	pragma comment(lib, "IPHLPAPI.lib")
#	pragma comment(lib, "ws2_32.lib")

#endif


#define INIT()		std::ios::sync_with_stdio(false); \
					std::cin.tie(NULL); \
					{ \
						auto t{ std::time(nullptr) }; \
						auto loc{ std::localtime(&t) }; \
						printf("start time: %04d-%02d-%02d %02d:%02d:%02d\n\n", 1900 + loc->tm_year, loc->tm_mon + 1, loc->tm_mday, loc->tm_hour, loc->tm_min, loc->tm_sec); \
					} \
					auto __startup_time__{ std::chrono::high_resolution_clock::now() }; \

#define FLUSH_OUT()	do { char ch; while (std::cin.readsome(&ch, 1) != 0); } while (0)

#define GET_CHAR()	do { char ch; while (std::cin.readsome(&ch, 1) != 0); getchar(); } while (0)

#define FINISH(n)	{ \
						auto __elapsed_time__{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - __startup_time__).count() / 1000. }; \
						printf("\nprogramm finished, exit code: %d \n", n); \
						printf("elapsed time: %3.3f ms \n", __elapsed_time__); \
					} \
					printf("press enter to exit... "); \
					GET_CHAR(); \
					return n

#define For(i, n)			for( std::remove_cv<std::remove_reference<decltype(n)>::type>::type (i) = (decltype(i))0; (i) < (n); ++(i) )
#define Forn(i, a, b)		for( std::remove_cv<std::remove_reference<decltype(a)>::type>::type (i) = (a); (i) < (b); ++(i) )
#define Fors(i, b, s)		for( std::remove_cv<std::remove_reference<decltype(b)>::type>::type (i) = (decltype(i))0; (i) < (b); (i) += (s) )
#define Forns(i, a, b, s)	for( std::remove_cv<std::remove_reference<decltype(a)>::type>::type (i) = (a); (i) < (b); (i) += (s) )
#define SET(t, v)			memset((t), (v), sizeof(t))

#define MAX(x, y)		((x) > (y) ? (x) : (y))
#define MIN(x, y)		((x) < (y) ? (x) : (y))

#define FP_EPSILON			1e-9
#define FP_EQUAL(x, y)		(((x) > (y) ? (x) - (y) : (y) - (x)) < FP_EPSILON)
#define FP_EQUAL_ZERO(x)	((x) > 0. ? (x) < FP_EPSILON : -FP_EPSILON < (x))
#define FP_GREATER_OR_EQUAL	(x, y)	((x) > (y) - FP_EPSILON)
#define FP_LESS_OR_EQUAL	(x, y)	((x) < (y) + FP_EPSILON)

#pragma comment(linker, "/STACK:200000000")

//      signed int					// -2e9  to 2e9   // (-2147483648)          - (2147483647)           // %d		// auto n = 1;		// int
typedef unsigned int		uint;	// 0     to 4e9   // (0)                    - (4294967295)           // %u		// auto n = 1u;		// unsigned int
typedef signed long long	int64;	// -9e18 to 9e18  // (-9223372036854775808) - (9223372036854775807)  // %I64d	// auto n = 1ll;	// long long
typedef unsigned long long	uint64;	// 0     to 18e18 // (0)                    - (18446744073709551615) // %I64u	// auto n = 1llu;	// unsigned long long
typedef std::vector<int>	VI;
typedef std::pair<int, int>	PII;

#endif // !_PCH_H_
