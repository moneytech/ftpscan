#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ftpscan.h"


int
connect_server(in_addr_t address, in_port_t port)
{
	struct sockaddr_in sin;
	int fd;
	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = address;

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket failed\n");
		exit(EXIT_FAILURE);
	}

	if(connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		error("connect failed");
		exit(EXIT_FAILURE);
	}

	return fd;
}


int
listen_port(in_port_t port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	int s;
	if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket() failed\n");
		return -1;
	}

	if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		error("bind() failed for port %d", port);
		return -1;
	}

	if(listen(s, 5) < 0) {
		fprintf(stderr, "listen() failed\n");
		return -1;
	}
	return s;
}

int
set_nonblocking(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return 0;
}
