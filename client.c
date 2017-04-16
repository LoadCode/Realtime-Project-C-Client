#include <stdio.h>
#include <pthread.h>
#include <uqeasysocket.h>
#include <dUQx.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 2222
#define SERVER_YES 2958

typedef enum{false, true} bool;

typedef struct
{
	int port;
	int channel;
	int resolution;
	int Ts;
	int finish;
	double vRef;
} data_t;

typedef struct
{
	int server_socket;
	int client_socket;
	unsigned int request;
	unsigned int inv_request;  //bit reversal request value
} client_t;

void * client_thread(void *args);


int main()
{
	// definición de los parámetros de dUQx
	data_t process_data;
	process_data.resolution = 1; //10 bits
	process_data.channel = 0; // A0 ADC input
	// creación del hilo
	pthread_t client_th;
	pthread_create(&client_th, NULL, client_thread, &process_data);
	pthread_join(client_th, NULL);
	printf("Saliendo de Main\n");
	return 0;
}


void * client_thread(void *args)
{
	data_t * process_data = (data_t *) args;
	client_t client_data;
	unsigned int server_request_answer = 0;
	double input_sample = 0.0;

	process_data->finish = 0;
	client_data.request = 45862;
	client_data.inv_request = client_data.request;
	swapbytes(&client_data.inv_request, sizeof(int));

	// Configuración del cliente
	if(client_create(IP, PORT, &client_data.server_socket))
	{
		printf("Error creando el cliente\n");
		return NULL;
	}

	// Iniciar la conexión
	sleep(2);
	send_data(client_data.server_socket, &server_request_answer, sizeof(int));
	swapbytes(&server_request_answer, sizeof(int));
	printf("server_request_answer = %d\n",server_request_answer);
	if(server_request_answer != SERVER_YES)
	{
		printf("Respuesta erronea del servidor\nterminado conexion y finalizando hilo\n");
		close_socket(client_data.server_socket);
		return NULL;
	}

	// Capturando parámetros del proceso
	receive_data(client_data.server_socket, &process_data->port, sizeof(int));
	swapbytes(&process_data->port, sizeof(int));
	receive_data(client_data.server_socket, &process_data->Ts, sizeof(int));
	swapbytes(&process_data->Ts, sizeof(int));

	// Inicialización de la tarjeta dUQx
	if(dUQx_Init(process_data->port))
	{
		printf("Problema configurand la tarjeta en el puerto %d\nFinalizando el cliente y el hilo\n",process_data->port);
		close_socket(client_data.server_socket);
		return NULL;
	}
	dUQX_SetResolution(process_data->resolution);
	dUQx_CalibrateAnalog(&process_data->vRef);

	// ciclo de captura y comunicación con servidor
	while(process_data->finish == 0)
	{
		dUQx_ReadAnalogSingle(process_data->channel, process_data->vRef, &input_sample);
		swapbytes(&input_sample, sizeof(double));
		send_data(client_data.server_socket, &input_sample, sizeof(double));
		receive_data(client_data.server_socket, &process_data->finish, sizeof(int));
		swapbytes(&process_data->finish, sizeof(int));
		sleep(process_data->Ts);
	}

	//finalización del cliente
	dUQx_End();
	close_socket(client_data.server_socket);
	return NULL;
}
