#include <getopt.h>
#include <limits.h>
#include <pthread.h>

#include "sha256.h"
#include "common.h"

void atender_cliente(int connfd);
void *thread(void *varg);

void print_help(char *command)
{
	printf("Servidor eco de mensajes.\n");
	printf("uso:\n %s <puerto>\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
}

bool seguir = true;

int main(int argc, char **argv)
{
	int opt;

	//Sockets
	int listenfd, *connfdp;
	unsigned int clientlen;
	//Direcciones y puertos
	struct sockaddr_in clientaddr;
	char *port;

	while ((opt = getopt (argc, argv, "h")) != -1){
		switch(opt)
		{
			case 'h':
				print_help(argv[0]);
				return 0;
			default:
				fprintf(stderr, "uso: %s <puerto>\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				return 1;
		}
	}

	if(argc != 2){
		fprintf(stderr, "uso: %s <puerto>\n", argv[0]);
		fprintf(stderr, "     %s -h\n", argv[0]);
		return 1;
	}else
		port = argv[1];

	//Valida el puerto
	int port_n = atoi(port);
	if(port_n <= 0 || port_n > USHRT_MAX){
		fprintf(stderr, "Puerto: %s invalido. Ingrese un número entre 1 y %d.\n", port, USHRT_MAX);
		return 1;
	}

	//Abre un socket de escucha en port
	listenfd = open_listenfd(port);

	if(listenfd < 0)
		connection_error(listenfd);

	printf("server escuchando en puerto %s...\n", port);

	pthread_t tid;
	while (seguir) {
		connfdp = malloc(sizeof(int));
		*connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		pthread_create(&tid, NULL, thread, connfdp);
	}

	return 0;
}

void *thread(void *varg)
{
	int connfd = *((int *)varg);
	pthread_detach(pthread_self());
	free(varg);
	atender_cliente(connfd);
	close(connfd);

	return NULL;
}

void atender_cliente(int connfd)
{
	int n;
	char buf[MAXLINE] = {0};

	SHA256_CTX ctx;
	BYTE sha256_buf[SHA256_BLOCK_SIZE];

	while(1){
		n = read(connfd, buf, MAXLINE);
		if(n <= 0)
			return;

		printf("Recibido: %s", buf);

		//Ignora '\n' asumiendo que esta al final de buf
		n--;

		//hash sha256
		sha256_init(&ctx);
		sha256_update(&ctx, (BYTE *) buf, n);
		sha256_final(&ctx, sha256_buf);

		//Reenvía el mensaje al cliente
		n = write(connfd, sha256_buf, SHA256_BLOCK_SIZE);
		if(n <= 0)
			return;

		memset(buf, 0, MAXLINE); //Encera el buffer
	}
}
