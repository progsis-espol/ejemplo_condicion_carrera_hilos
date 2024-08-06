# Ejemplo condiciones de carrera con hilos
Este repositorio contiene código ejemplo que permite la demostración de los efectos de condiciones de carrera en una aplicación cliente - servidor multihilo. En este ejemplo, el usuario ingresa una línea de texto desde el cliente, el cliente la envia al servidor, el servido calcula el hash *sha256* de la cadena de texto y retorna este resultado en formato binario al cliente. El código de esta aplicación está basado en el capítulo 11 de [Computer Systems: A Programmer's Perspective](http://csapp.cs.cmu.edu/3e/home.html).

## Uso
Para ejecutar el servidor se debe especificar como argumento el puerto TCP, por ejemplo:
```
./server 8080
server escuchando en puerto 8080...
```

Asumiendo que el servidor esta corriendo en una maquina con la IP 192.168.100 en el puerto 8080, ejemplo de ejecución del cliente:
```
./client 192.168.100 8080
Conectado exitosamente a 192.168.100 en el puerto 8080.
Ingrese texto para enviar al servidor, Ctrl+c para terminar...
> 
```

El servidor ignora el salto de línea al final de la cadena de caracteres enviada por el cliente. Ejemplo de funcionamiento correcto:
```
./client 192.168.100 8080
Conectado exitosamente a 192.168.100 en el puerto 8080.
Ingrese texto para enviar al servidor, Ctrl+c para terminar...
> hola
b221d9dbb083a7f33428d7c2a3c3198ae925614d70210e28716ccaa7cd4ddb79
> 
```

Si el servidor esta en la misma máquina, entonces es necesario abrir otra ventana/tab de terminal y ejecutar el cliente de esta forma:
```
$ ./client 127.0.0.1 8080
Conectado exitosamente a 127.0.0.1 en el puerto 8080.
Ingrese texto para enviar al servidor, Ctrl+c para terminar...
> 
```

El código en [test.c](test.c) envia la palabra "*test*" al servidor y comprueba si el servidor responde con el hash correcto. Muestra "*OK*" si es correcto e inmediatamente vuelve a enviar un nuevo "*test*". Muestra "*BOOM*" y se desconecta del servidor si es incorrecto.

## Demostración

Existen varias formas de provocar una condición de carrera en esta aplicación. La primera eliminando el uso de memoria dinámica para almacenar el descriptor de archivo del socket de conexión en el servidor [server.c](server.c):

```C
 68         pthread_t tid;                                                                                         
 69         while (seguir) {                                                                                       
 70                 connfdp = malloc(sizeof(int));                                                                 
 71                 *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);                       
 72                 pthread_create(&tid, NULL, thread, connfdp);                                                   
 73         }  
```

puede modificarse en las líneas 70 - 72 a:

```C
 68         pthread_t tid;                                                                                         
 69         while (seguir) {                                                                                       
 70                 int connfd;                                                                 
 71                 connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);                       
 72                 pthread_create(&tid, NULL, thread, &connfd); //Condición de carrera                                                  
 73         }
```

otra forma de condición de carrera es modificando *void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)* en [sha256.c](sha256.c) para que no sea reentrante:

```C
 99 void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)                                             
100 {                                                                                                              
101         WORD i;                                                                                                
102                                                                                                                
103         for (i = 0; i < len; ++i) {                                                                            
104                 ctx->data[ctx->datalen] = data[i];                                                             
105                 ctx->datalen++;  
```

puede modificarse en la línea 101 a:

```C
 99 void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)                                             
100 {                                                                                                              
101         static WORD i;       //Condición de carrera                                                                                           
102                                                                                                                
103         for (i = 0; i < len; ++i) {                                                                            
104                 ctx->data[ctx->datalen] = data[i];                                                             
105                 ctx->datalen++;  
```

Para observar el efecto de las condiciones de carrera es necesario conectar varios clientes *test* de manera simultanea al servidor.

## Compilación
Para compilar cliente, servidor y test:
```
$ make
```
Para compilar solo el servidor:
```
$ make server
```
Para compilar cliente y servidor facilitando la depuración con gdb:
```
$ make debug
```
Para compilar cliente y servidor habilitando la herramienta AddressSanitizer, facilita la depuración en tiempo de ejecución:
```
$ make sanitize
```
