/* vim: set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab : */

/**
 * main.c
 *
 * Simple UDP i IPC server.
 * Insert message received over network (udp) in System V message queue.
 *
 */

#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MYPORT "5555"  // port for network
#define MAXBUFLEN 1024 // max buffer ipc and server

/* Flag set by `--verbose'. */
static int verbose_flag = 0;

// structure data send to queue
struct my_msgbuf {
	long mtype;
	char mtext[MAXBUFLEN];
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	// option management
	// for now one argument ...
	if(argc > 0) {
		int i;
		for(i=1; i < argc; i++) {
			if(strcmp(argv[i], "--verbose") == 0) {
				verbose_flag = 1;
				printf("listener: verbose set \n");
			} else {
				printf("Undefined args : %s \n", argv[i]);
				exit(1);
			}
		}
	}

	key_t qkey;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	int qmsqid;
	int sockfd;
	int rv;
	int numbytes;
	struct my_msgbuf qbuf;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;

	// default message type
	qbuf.mtype = 25;

	// generate token for ipc
	if ((qkey = ftok("main.c", 'B')) == -1) {
		perror("ftok");
		exit(1);
	}

	// get new msgid for ipc
	if ((qmsqid = msgget(qkey, 0644 | IPC_CREAT)) == -1) {
		perror("msgget");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_UNSPEC to IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		// find ip to bind to ?
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		// make listening
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	if(verbose_flag) {
		printf("listener: waiting connection...\n");
		printf("listener: msgpid %i\n", qmsqid);
	}

	addr_len = sizeof their_addr;

	while((numbytes = recvfrom(sockfd, qbuf.mtext, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr,
				&addr_len)) != -1) {

		int len = strlen(qbuf.mtext);

		if (qbuf.mtext[len-1] == '\n') {
			qbuf.mtext[len-1] = '\0';
		}

		qbuf.mtext[numbytes] = '\0';

		if(verbose_flag) {
			printf("listener: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s,
				sizeof s
			));
			printf("listener: packet is %d bytes long\n", numbytes);
			printf("listener: packet contains \"%s\" ", qbuf.mtext);
			printf("\n");
		}

		if (msgsnd(qmsqid, &qbuf, len+1, 0) == -1) {
			perror("msgsnd");
			exit(1);
		}

		bzero(qbuf.mtext, sizeof qbuf.mtext);
	}

	if (msgctl(qmsqid, IPC_RMID, NULL) == -1) {
		perror("msgctl");
		exit(1);
	}

	close(sockfd);

	return 0;
}
