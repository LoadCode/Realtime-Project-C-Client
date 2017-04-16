#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <dUQx.h>
#include <uqeasysocket.h>

#define IP_SERVER "127.0.0.1"
#define PORT 2222
#define REQUEST_VAL 45862
#define REQUEST_YES 2958

struct socket_info
{
	int server_socket;
	int client_socket;
};

typedef struct socket_info socket_info_t;
typedef struct timespec time_process_t;

typedef struct 
{
	int usbPort;
	int analogInput;
	int resolution;
	double vref;
	double normal_ts;
	time_process_t sample_time;	
} process_data_t;

time_process_t get_time_struct(double Ts);

int main()
{
	socket_info_t client_info;
	process_data_t process_info;
	int request = REQUEST_VAL;
	int req_answer;
	int finish = 0;
	double time_counter = 0.0;
	double time_counter_swap = 0.0;
	double signal_value = 0.0;
	double signal_swap = 0.0;
	
	// algunas inicilizaciones
	process_info.analogInput = 0; //canal A0 de la tarjeta Arduino
	process_info.resolution = 1;  //for 10 bit resolution

	// creates client
	if(client_create(IP_SERVER, PORT, &client_info.server_socket))
	{
		printf("Error creando el cliente\n");
		return 0;
	}

	// try to connect with server
	swapbytes(&request, sizeof(int));
	printf("enviando solicitud\n");
	send_data(client_info.server_socket, &request, sizeof(int));
	receive_data(client_info.server_socket, &req_answer, sizeof(int));
	swapbytes(&req_answer, sizeof(int));
	if(req_answer == REQUEST_YES)
		printf("Respuesta request: OK\n");
	else
		printf("Respuesta request: FAIL (leaving thread)\n");
	if(req_answer != REQUEST_YES)
		return 0;
	
	// Obteniendo parámetros
	receive_data(client_info.server_socket, &process_info.usbPort, sizeof(int));
	swapbytes(&process_info.usbPort, sizeof(int));
	receive_data(client_info.server_socket, &process_info.normal_ts, sizeof(double));
	swapbytes(&process_info.normal_ts, sizeof(double));
	
	/*printf("Ts = %f\n",process_info.normal_ts);
	printf("usbPort = %d\n",process_info.usbPort);*/
	
	//Inicialización de dUQx
	if(dUQx_Init(process_info.usbPort))
	{
		printf("Error iniciando dUQx\n");
		close_socket(client_info.server_socket);
		return 0;
	}
	dUQx_CalibrateAnalog(&process_info.vref);
	dUQX_SetResolution(process_info.resolution);

	process_info.sample_time = get_time_struct(process_info.normal_ts);
	
	// Muestreo del proceso
	do
	{
		dUQx_ReadAnalogSingle(process_info.analogInput, process_info.vref, &signal_value);
		signal_swap = signal_value;
		swapbytes(&signal_swap, sizeof(double));
		send_data(client_info.server_socket, &signal_swap, sizeof(double));
		time_counter_swap = time_counter;
		swapbytes(&time_counter_swap, sizeof(double));
		send_data(client_info.server_socket, &time_counter_swap, sizeof(double));
		receive_data(client_info.server_socket, &finish, sizeof(int));
		swapbytes(&finish, sizeof(int));
		printf("signal_value = %f\n",signal_value);
		nanosleep(&process_info.sample_time, NULL);
		time_counter += process_info.normal_ts; // seconds
	}while(finish == 0);
	close_socket(client_info.server_socket);
	dUQx_End();
	printf("Proceso de cliente terminado bien\n");
	return 0;
}


time_process_t get_time_struct(double Ts)
{
	time_process_t time;
	double seconds, nanoseconds;
	nanoseconds  = modf(Ts, &seconds) * 1000000000UL; // gets seconds and nanoseconds from incomming millis
	time.tv_sec  = seconds;
	time.tv_nsec = nanoseconds;
	return time;
}


