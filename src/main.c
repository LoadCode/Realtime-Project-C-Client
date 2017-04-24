#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <dUQx.h>
#include <uqeasysocket.h>	
#include <utils.h>
#include <string.h>

#define IP_SERVER   "127.0.0.1"
#define MAIN_PORT   34869
#define PORT_CLIENT 34869
#define REQUEST_VAL 45862
#define REQUEST_YES 2958
#define MAX_OUTPUT 5 //5 volts
#define MIN_OUTPUT 0 //0 volts


pthread_mutex_t lockRead;
pthread_mutex_t lockWrite;

void GetControlSignal(control_data_t * controlData)
{
	double dProcessOut;
	dProcessOut = controlData->processOutput - controlData->lastProcessOutput;
	controlData->error = controlData->setpoint - controlData->processOutput;
	controlData->iTerm += controlData->ki * controlData->error;

	// antiWindUp
	if(controlData->iTerm > MAX_OUTPUT)
		controlData->iTerm = MAX_OUTPUT;
	else if(controlData->iTerm < MIN_OUTPUT)
		controlData->iTerm = MIN_OUTPUT;

	controlData->controllerOutput = controlData->kp  * controlData->error
								  + controlData->iTerm
								  - controlData->kd * dProcessOut;

	// antiWindUp on ouput (anti-saturation)
	if(controlData->controllerOutput > MAX_OUTPUT)
		controlData->controllerOutput = MAX_OUTPUT;
	else if(controlData->controllerOutput < MIN_OUTPUT)
		controlData->controllerOutput = MIN_OUTPUT;
}


void * ControllerI(void * args)
{
	printf("Iniciando HILO I.\n");
	control_data_t *controlInfo = (control_data_t*)args;
	client_data_t client;

	controlInfo->currentTime = 0;
	strcpy(client.ip, IP_SERVER);
	client.port = PORT_CLIENT;
	controlInfo->sampleTime = GetTimeStruct(controlInfo->normalTs);

	// creación del primer cliente
	if(client_create(client.ip, client.port, &client.serverSocket))
	{
		printf("Error al intentar conectar uno de los hilos con el servidor: %s \n",client.ip);
		return NULL;
	}
	printf("Cliente ctrl I creado\n");

	while(controlInfo->finishFlag == false)
	{
		// Lee la entrada del proceso a través de dUQx
		pthread_mutex_lock(&lockRead);
		dUQx_ReadAnalogSingle(controlInfo->analogInput, controlInfo->refVolt, &controlInfo->processOutput);
		pthread_mutex_unlock(&lockRead);

		// Calcula la ley de control para este hilo
		GetControlSignal(controlInfo);

		// Escribe la tensión entregada por el controlador
		pthread_mutex_lock(&lockWrite);
		dUQx_WriteAnalog(controlInfo->controllerOutput, controlInfo->refVolt, controlInfo->analogOutput);
		pthread_mutex_unlock(&lockWrite);

		// Se envía la señal que entrega la planta
		controlInfo->swapProcessOutput = controlInfo->processOutput;
		swapbytes(&controlInfo->swapProcessOutput, sizeof(double));
		send_data(client.serverSocket, &controlInfo->swapProcessOutput, sizeof(double));

		// Envía la señal calculada por el controlador
		controlInfo->swapControlOutput = controlInfo->controllerOutput;
		swapbytes(&controlInfo->swapControlOutput, sizeof(double));
		send_data(client.serverSocket, &controlInfo->swapControlOutput, sizeof(double));

		// sends the time value
		controlInfo->currentTimeSwap = controlInfo->currentTime;
		swapbytes(&controlInfo->currentTimeSwap, sizeof(double));
		send_data(client.serverSocket, &controlInfo->currentTimeSwap, sizeof(double));

		controlInfo->currentTime += controlInfo->normalTs;

		// Actualiza el valor de la bandera de finalización del ciclo
		receive_data(client.serverSocket, &controlInfo->finishFlag, sizeof(int));
		swapbytes(&controlInfo->finishFlag, sizeof(int));
		nanosleep(&controlInfo->sampleTime, NULL);
	}
	close_socket(client.serverSocket);
	printf("Terminado el hilo I\n");
	return NULL;
}


void * ControllerII(void * args)
{
	printf("Iniciando HILO II.\n");
	control_data_t *controlInfo = (control_data_t*)args;
	client_data_t client;

	controlInfo->currentTime = 0;
	strcpy(client.ip, IP_SERVER);
	client.port = PORT_CLIENT;
	controlInfo->sampleTime = GetTimeStruct(controlInfo->normalTs);

	// creación del primer cliente
	if(client_create(client.ip, client.port, &client.serverSocket))
	{
		printf("Error al intentar conectar uno de los hilos con el servidor: %s \n",client.ip);
		return NULL;
	}
	printf("Cliente ctrl II creado\n");

	while(controlInfo->finishFlag == false)
	{
		// Lee la entrada del proceso a través de dUQx
		pthread_mutex_lock(&lockRead);
		dUQx_ReadAnalogSingle(controlInfo->analogInput, controlInfo->refVolt, &controlInfo->processOutput);
		pthread_mutex_unlock(&lockRead);

		// Calcula la ley de control para este hilo
		GetControlSignal(controlInfo);

		// Escribe la tensión entregada por el controlador
		pthread_mutex_lock(&lockWrite);
		dUQx_WriteAnalog(controlInfo->controllerOutput, controlInfo->refVolt, controlInfo->analogOutput);
		pthread_mutex_unlock(&lockWrite);

		// Se envía la señal que entrega la planta
		controlInfo->swapProcessOutput = controlInfo->processOutput;
		swapbytes(&controlInfo->swapProcessOutput, sizeof(double));
		send_data(client.serverSocket, &controlInfo->swapProcessOutput, sizeof(double));

		// Envía la señal calculada por el controlador
		controlInfo->swapControlOutput = controlInfo->controllerOutput;
		swapbytes(&controlInfo->swapControlOutput, sizeof(double));
		send_data(client.serverSocket, &controlInfo->swapControlOutput, sizeof(double));

		// sends the time value
		controlInfo->currentTimeSwap = controlInfo->currentTime;
		swapbytes(&controlInfo->currentTimeSwap, sizeof(double));
		send_data(client.serverSocket, &controlInfo->currentTimeSwap, sizeof(double));

		controlInfo->currentTime += controlInfo->normalTs;

		// Actualiza el valor de la bandera de finalización del ciclo
		receive_data(client.serverSocket, &controlInfo->finishFlag, sizeof(int));
		swapbytes(&controlInfo->finishFlag, sizeof(int));
		nanosleep(&controlInfo->sampleTime, NULL);
	}
	close_socket(client.serverSocket);
	printf("Terminado el hilo II\n");
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
	processOne.finishFlag   = false;
	processTwo.finishFlag   = false;
	processOne.resolution   = 1; // Resolución de 10 bits para el ADC
	processTwo.resolution   = 1; // Resolución de 10 bits para el ADC
	processOne.analogInput  = 0; // Arduino A0 pin
	processTwo.analogInput  = 1; // Arduino A1 pin
	processOne.analogOutput = 9; // First controller's output in Arduino pwm pin 9
	processTwo.analogOutput = 10;// First controller's output in Arduino pwm pin 10
	processOne.iTerm        = 0.0;
	processTwo.iTerm        = 0.0;
	processOne.lastProcessOutput = 0;
	processTwo.lastProcessOutput = 0;
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
	printf("Controlador 1\n");
	ShowParameters(&processOne);
	printf("Controlador 2\n");
	ShowParameters(&processTwo);
	// Inicialización de dUQx
	if(dUQx_Init(processOne.usbPort))
	{
		printf("Error iniciando dUQx\n");
		close_socket(clientI.serverSocket);
		return 0;
	}
	dUQx_CalibrateAnalog(&processOne.refVolt);
	dUQX_SetResolution(processOne.resolution);
	processTwo.refVolt = processOne.refVolt;
	processOne.sampleTime = GetTimeStruct(processOne.normalTs);
	processTwo.sampleTime = GetTimeStruct(processTwo.normalTs);

	// Creación de hilos por proceso
	pthread_create(&thControlI, NULL, ControllerI, &processOne);
	pthread_create(&thControlII, NULL, ControllerII, &processTwo);
	pthread_join(thControlI, NULL);
	pthread_join(thControlII, NULL);

	// Free resources
	pthread_mutex_destroy(&lockRead);
	pthread_mutex_destroy(&lockWrite);
	close_socket(clientI.serverSocket);
	printf("salimos bien\n");
	return 0;
}


