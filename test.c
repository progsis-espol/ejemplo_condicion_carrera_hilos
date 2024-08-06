#include "sha256.h"
#include "common.h"
#include <stdbool.h>

bool compare_sha256(BYTE *a, BYTE *b)
{
	for(int i = 0; i < SHA256_BLOCK_SIZE; i++)
		if(a[i] != b[i])
			return false;

	return true;
}

/* Para ejecutar este test correctamente,
 * ejecutar sha_server con opción -n */
int main(int argc, char **argv)
{
	int clientfd;
	char *port;
	char *host;

	//SHA256 de "test"
	BYTE answer_test[] = {0x9f,0x86,0xd0,0x81,0x88,0x4c,0x7d,0x65,0x9a,0x2f,0xea,0xa0,0xc5,0x5a,0xd0,0x15,0xa3,0xbf,0x4f,0x1b,0x2b,0x0b,0x82,0x2c,0xd1,0x5d,0x6c,0x15,0xb0,0xf0,0x0a,0x08};
	BYTE buffer[SHA256_BLOCK_SIZE];
	
	bool go = true;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}
	host = argv[1];
	port = argv[2];

	clientfd = open_clientfd(host, port);

	int n = 0;
	while(go)
	{
		n = write(clientfd, "test\n", 5);
		if(n < 0)
			break;

		n = read(clientfd, buffer, SHA256_BLOCK_SIZE);
		if(n < 0)
			break;
			
		if((go = compare_sha256(buffer,answer_test)))
			printf("OK\n");
		else
			fprintf(stderr, "BOOM ERROR!\n");
	}

	printf("Desconectando...\n");
	close(clientfd);
	exit(0);
}
