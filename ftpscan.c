#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#include "ftpscan.h"

static int test_port(int fd, in_port_t);
static void run_scan();

static in_addr_t local_address;
static in_addr_t target_address;
static in_port_t target_port = 21;

static void
usage(const char *progname)
{
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	int ch;
	char *progname = argv[0];

	while((ch = getopt(argc, argv, "a:p:v")) != -1) {
		switch(ch) {
			case 'v':
				enable_debug(1);
				break;
			case 'p':
				target_port = atoi(optarg);
				break;
			case 'a':
				local_address = inet_addr(optarg);
				break;
			case '?':
			default:
				usage(argv[0]);
		}
	}
	argc -= optind;
	argv += optind;

	if(argc < 2)
		usage(progname);

	target_address = inet_addr(argv[0]);
	ports_initialize(argv[1]);
	run_scan();
	return 0;
}

static void
run_scan()
{
	int fd = connect_server(target_address, target_port);
	if(fd < 0)
		exit(EXIT_FAILURE);
	if(ftp_anon_login(fd)) {
		warn("Login failed.");
		close(fd);
		exit(EXIT_FAILURE);
	}
	info("Logged in.");
	int i;
	for(i = next_port(); i != 0; i = next_port())
		test_port(fd, i);
	ftp_exchange_command(fd, "QUIT");
}

static int
test_port(int fd, in_port_t port)
{


	char buffer[256];
	snprintf(buffer, 256, "EPRT |1|192.168.1.102|%u|", port);
	int code = ftp_exchange_command(fd, buffer);
	if(!ftp_code_okay(code)) 
		return -1;
	int s = listen_port(port);
	code = ftp_exchange_command(fd, "LIST");
	if(code != 150) {
		warn("Unexpected respose code from LIST: %d", code);
	}


	fprintf(stderr, "[+] Testing port %d", port);
	set_nonblocking(s);

	struct pollfd pfd;
	pfd.fd = s;
	pfd.events = POLLIN;
	pfd.revents = 0;

	if(poll(&pfd, 1, 2000) == 0) {
		close(s);
		fprintf(stderr, " TIME OUT\n");
		return -1;
	}
	if((pfd.revents & POLLIN) == 0) {
		close(s);
		warn("wtf");
		return -1;
	}

	struct sockaddr_in sin2;
	socklen_t len = sizeof(sin2);

	int s2;
	if((s2 = accept(s, (struct sockaddr *)&sin2, &len)) < 0) {
		error("accept() failed()");
		close(s);
		return -1;
	}

	fprintf(stderr, " ACCEPTED!\n");
	close(s);
	drain_all(s2);
	ftp_command_response(fd);
	return 0;
}



