/*Biblioteca uqeasysocket para la comunicación mediante sockets
  entre clientes y servidores TCP.

  version: 1.1: Soporte para Linux.

  Alexander Lopez Parrado (2009)
  */


#ifndef _UQEASYSOCKET_H_
#define _UQEASYSOCKET_H_

#include <stdio.h>

/***********************Listado de funciones**************************************/


void swapbytes(void *_object, size_t _size);


int client_create(const char * server_ip,unsigned short port,int *server_sock);
/*Crea un cliente y se conecta con un servidor.

  server_ip: es una cadena con la dirección ip del servidor ej: "192.168.1.100"
  port: es el puerto del servidor al cual se desea conectar el cliente.
  server_sock: puntero en donde se desea almacenar el identificador del socket del servidor.

  retorna 0 si no hubo error.

  precondición:Niguna.

  postcondición: Socket creado para cliente.
*/






int server_create(unsigned short port,int *server_sock);
/*Crea un servidor.

  port: puerto que abrirá el servidor.
  sever_sock: puntero en donde se desea almacenar el identificador del socket del servidor.

  retorna 0 si no hubo error.

  precondición:Niguna.

  postcondición: Socket creado para servidor.
*/







int client_wait(int server_sock,int *client_sock);
/*Espera la conexión de un cliente.

  server_sock: identificador del socket del servidor sobre el cual se espera la conexión.
  client_sock: puntero en donde se desea almacenar el identificador del socket del
  cliente que estableció la conexión.

  retorna 0 si no hubo error.

  precondición: servidor creado.

  postcondición: Socket creado para cliente que estableció conexión.
*/






int receive_data(int sock,void * buffer,int len);
/*Espera datos de un socket.

  sock: identificador del socket desde el cual se leerán los datos.
  buffer: puntero a la zona de memoria donde se almacenarán los bytes leídos.
  len: cantidad de bytes a leer.

  retorna la cantidad de bytes recibidos si no hubo error.

  precondición: socket (cliente,servidor) creado.

  postcondición: Ninguna.
*/






int send_data(int sock,const void * buffer,int len);
/*Envía datos a un socket.

  sock: identificador del socket hacía donde se enviarán los datos.
  buffer: puntero a la zona de memoria de donde se leerán los bytes.
  len: cantidad de bytes a enviar.

  retorna la cantidad de bytes enviados si no hubo error.

  precondición: socket (cliente,servidor) creado .

  postcondición: Ninguna.
*/



int close_socket(int sock);
/*Cierra un socket.

  sock: identificador del socket que se desea cerrar.

  retorna 0 si no hubo error.

  precondición: socket (cliente,servidor) creado .

  postcondición: Socket cerrado.
  */


#endif
