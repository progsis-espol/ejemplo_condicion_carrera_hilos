#include <getopt.h>
#include <limits.h>

#include "sha256.h"
#include "common.h"

void print_help(char *command)
{
	printf("Cliente simple de envio de mensajes.\n");
	printf("uso:\n %s <hostname> <puerto>\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
}

//Imprime cadena de bytes en formato hexadecimal
void print_hex(const BYTE* data, size_t size)
{
    int i;
    for(i = 0; i < size; ++i)
        printf("%02x", data[i]);

    printf("\n");
}

int main(int argc, char **argv)
{
	int opt;

	//Socket
	int clientfd;
	//Direcciones y puertos
	char *hostname, *port;

	//Lectura desde consola
	char *linea_consola;
	BYTE sha256_buf[SHA256_BLOCK_SIZE] = {0};
	size_t max = MAXLINE;
	ssize_t n, l = 0;

	while ((opt = getopt (argc, argv, "h")) != -1){
		switch(opt)
		{
			case 'h':
				print_help(argv[0]);
				return 0;
			default:
				fprintf(stderr, "uso: %s <hostname> <puerto>\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				return 1;
		}
	}

	int contador = 0;
	int index = 0;
	for (index = optind; index < argc; index++)
		contador++;

	if(contador != 2){
		fprintf(stderr, "uso: %s <hostname> <puerto>\n", argv[0]);
		fprintf(stderr, "     %s -h\n", argv[0]);
		return 1;
	}else{
		hostname = argv[optind];
		port = argv[optind + 1];
	}

	//Valida el puerto
	int port_n = atoi(port);
	if(port_n <= 0 || port_n > USHRT_MAX){
		fprintf(stderr, "Puerto: %s invalido. Ingrese un número entre 1 y %d.\n", port, USHRT_MAX);
		return 1;
	}

	//Se conecta al servidor retornando un socket conectado
	clientfd = open_clientfd(hostname, port);

	if(clientfd < 0)
		connection_error(clientfd);

	printf("Conectado exitosamente a %s en el puerto %s.\n", hostname, port);

	linea_consola = (char *) calloc(1, MAXLINE);
	printf("Ingrese texto para enviar al servidor, Ctrl+c para terminar...\n");
	printf("> ");
	l = getline(&linea_consola, &max, stdin); //lee desde consola
	while(l > 0){
		n = write(clientfd, linea_consola, l); //Envia al servidor
		if(n<=0)
			break;

		n = read(clientfd, sha256_buf, SHA256_BLOCK_SIZE); //Lee respuesta del servidor
		if(n<=0)
			break;

		print_hex(sha256_buf, SHA256_BLOCK_SIZE);

		//Volver a leer desde consola
		printf("> ");
		l = getline(&linea_consola, &max, stdin);
	}


	printf("Desconectando...\n");
	free(linea_consola);
	close(clientfd);

	return 0;
}