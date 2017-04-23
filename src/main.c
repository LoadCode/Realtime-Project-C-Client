#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <dUQx.h>
#include <uqeasysocket.h>	
#include <utils.h>
#include <string.h>

#define IP_SERVER "127.0.0.1"
#define MAIN_PORT 34869
#define PORT_CLIENT  34869
#define REQUEST_VAL 45862
#define REQUEST_YES 2958


void * ControllerI(void * args)
{
	printf("Iniciando HILO I.\n");
	control_data_t *controlInfo = (control_data_t*)args;
	client_data_t client;

	controlInfo->currentTime = 0;
	double signal_swap = 4;
	double control_swap = 5;
	strcpy(client.ip, IP_SERVER);
	client.port = PORT_CLIENT;
	controlInfo->sampleTime = GetTimeStruct(controlInfo->normalTs);
	swapbytes(&signal_swap, sizeof(double));
	swapbytes(&control_swap, sizeof(double));

	// creación del primer cliente
	if(client_create(client.ip, client.port, &client.serverSocket))
	{
		printf("Error al intentar conectar uno de los hilos con el servidor: %s \n",client.ip);
		return NULL;
	}
	printf("Cliente  ctrl I creado\n");
	while(controlInfo->finishFlag == false)
	{
		//Updates the status of the FINISH flag from the java server
		send_data(client.serverSocket, &signal_swap, sizeof(double));
		send_data(client.serverSocket, &control_swap, sizeof(double));
		controlInfo->normalTsSwaped = controlInfo->currentTime;
		swapbytes(&controlInfo->normalTsSwaped, sizeof(double));
		send_data(client.serverSocket, &controlInfo->normalTsSwaped, sizeof(double));
		receive_data(client.serverSocket, &controlInfo->finishFlag, sizeof(int));
		swapbytes(&controlInfo->finishFlag, sizeof(int));
		controlInfo->currentTime += controlInfo->normalTs;
		nanosleep(&controlInfo->sampleTime, NULL);
	}
	close_socket(client.serverSocket);
	printf("Terminado el hilo\n");
	return NULL;
}


void * ControllerII(void * args)
{
	printf("Iniciando HILO II.\n");
	control_data_t *controlInfo = (control_data_t*)args;
	client_data_t client;

	controlInfo->currentTime = 0;
	double signal_swap = 6;
	double control_swap = 7;
	strcpy(client.ip, IP_SERVER);
	client.port = PORT_CLIENT;
	controlInfo->sampleTime = GetTimeStruct(controlInfo->normalTs);
	swapbytes(&signal_swap, sizeof(double));
	swapbytes(&control_swap, sizeof(double));
	// creación del primer cliente
	if(client_create(client.ip, client.port, &client.serverSocket))
	{
		printf("Error al intentar conectar uno de los hilos con el servidor: %s \n",client.ip);
		return NULL;
	}
	printf("Cliente ctrl II creado\n");
	while(controlInfo->finishFlag == false)
	{
		//Updates the status of the FINISH flag from the java server
		send_data(client.serverSocket, &signal_swap, sizeof(double));
		send_data(client.serverSocket, &control_swap, sizeof(double));
		controlInfo->normalTsSwaped = controlInfo->currentTime;
		swapbytes(&controlInfo->normalTsSwaped, sizeof(double));
		send_data(client.serverSocket, &controlInfo->normalTsSwaped, sizeof(double));
		controlInfo->currentTime += controlInfo->normalTs;
		receive_data(client.serverSocket, &controlInfo->finishFlag, sizeof(int));
		swapbytes(&controlInfo->finishFlag, sizeof(int));
		nanosleep(&controlInfo->sampleTime, NULL);
	}
	close_socket(client.serverSocket);
	printf("Terminado el hilo\n");
	return NULL;
}



int main()
{
	client_data_t clientI;
	control_data_t processOne, processTwo;
	pthread_t thControlI;
	pthread_t thControlII;


	// Inicializaciones
	clientI.port = PORT_CLIENT;
	clientI.requestValue = REQUEST_VAL;
	strcpy(clientI.ip, IP_SERVER);
	processOne.finishFlag = false;
	processTwo.finishFlag = false;

	// creación del primer cliente
	if(client_create(clientI.ip, clientI.port, &clientI.serverSocket))
	{
		printf("Error al intentar conectar con el servidor: %s \n",clientI.ip);
		return 1;
	}

	// intentamos establecer conexión con el servidor
	swapbytes(&clientI.requestValue, sizeof(int));
	send_data(clientI.serverSocket, &clientI.requestValue, sizeof(int));
	receive_data(clientI.serverSocket, &clientI.serverAnswer, sizeof(int));
	swapbytes(&clientI.serverAnswer, sizeof(int));

	if(clientI.serverAnswer == REQUEST_YES)
	{
		printf("Respuesta request: OK\n");
	}
	else
	{
		printf("Respuesta del servidor: FAIL (leaving thread)\n");
		return 1;
	}

	// Obtención de parámetros
	GettingParameters(&clientI, &processOne, &processTwo);

	// Inicialización de dUQx

	// Creación de hilos por proceso
	printf("Controlador 1\n");
	ShowParameters(&processOne);
	printf("Controlador 2\n");
	ShowParameters(&processTwo);

	pthread_create(&thControlI, NULL, ControllerI, &processOne);
	pthread_create(&thControlII, NULL, ControllerII, &processTwo);
	pthread_join(thControlI, NULL);
	pthread_join(thControlII, NULL);

	close_socket(clientI.serverSocket);
	printf("salimos bien\n");
	return 0;
}










/*
void sampler_control_handler();

int main()
{
	socket_info_t client_info;
	process_data_t process_info;
	int request = REQUEST_VAL;
	int req_answer;
	int finish 				  = 0;
	process_info.current_time = 0.0;
	double time_counter_swap  = 0.0;
	double signal_value       = 0.0;
	double signal_swap 		  = 0.0;
	timer_t timer_sampler;
	struct itimerspec timer_params;
	struct sigevent event;
	struct sigaction action;
	int signum = SIGALRM;
	sigset_t set;
	

	// algunas inicilizaciones
	process_info.analogInput = 0; //canal A0 de la tarjeta Arduino
	process_info.resolution  = 1; //for 10 bit resolution
	sigemptyset(&set);
	sigaddset(&set, signum);

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
	receive_data(client_info.server_socket, &process_info.setpoint, sizeof(double));
	swapbytes(&process_info.setpoint, sizeof(double));
	receive_data(client_info.server_socket, &process_info.step_time, sizeof(double));
	swapbytes(&process_info.step_time, sizeof(double));
	printf("Ts = %f\n",process_info.normal_ts);
	
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
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo  = signum;
	sigprocmask(SIG_BLOCK, &set, NULL);
	action.sa_handler = &sampler_control_handler;
	action.sa_flags   = 0;
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	sigaction(signum, &action, NULL);
	timer_create(CLOCK_REALTIME, &event, &timer_sampler);
	//Configuración de parámetros del temporizador
	timer_params.it_value.tv_sec     = process_info.sample_time.tv_sec;
	timer_params.it_value.tv_nsec    = process_info.sample_time.tv_nsec;
	timer_params.it_interval.tv_sec  = process_info.sample_time.tv_sec;
	timer_params.it_interval.tv_nsec = process_info.sample_time.tv_nsec;

	// Muestreo del proceso (puede ser incluido en un hilo o en una tarea temporizada)
	do
	{
		//Reads one sample & sends it to server
		dUQx_ReadAnalogSingle(process_info.analogInput, process_info.vref, &signal_value);
		signal_swap = signal_value;
		swapbytes(&signal_swap, sizeof(double));
		send_data(client_info.server_socket, &signal_swap, sizeof(double));
		//Computes the control signal & sends it to server
		get_control_signal(&process_info);
		send_data(client_info.server_socket, &process_info.control_signal_swap, sizeof(double));
		//Sends the value of the time variable to the server
		time_counter_swap = process_info.current_time;
		swapbytes(&time_counter_swap, sizeof(double));
		send_data(client_info.server_socket, &time_counter_swap, sizeof(double));
		//Updates the status of the FINISH flag from the java server
		receive_data(client_info.server_socket, &finish, sizeof(int));
		swapbytes(&finish, sizeof(int));
		//Updates the value of the time variable
		process_info.current_time += process_info.normal_ts; // seconds
		//waits for the next iteration
		nanosleep(&process_info.sample_time, NULL);
	}while(finish == 0);
	close_socket(client_info.server_socket);
	dUQx_End();
	printf("Proceso de cliente terminado bien\n");
	return 0;
}


void sampler_control_handler()
{
	printf("Inside loop\n");
}

*/
