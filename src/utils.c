#include <utils.h>
#include <stdio.h>
#include <math.h>


void GettingParameters(client_data_t *client, control_data_t *controllerOne, control_data_t *controllerTwo)
{

	/*Orden del paso de parámetros desde el servidor
	 * CONTROLLER ONE
	 * usbPort
	 * Ts
	 * setpoint
	 * kp
	 * ki
	 * kd
	 *
	 * CONTROLLER TWO
	 * Ts
	 * setpoint
	 * kp
	 * ki
	 * kd
	 * */

	// Controller One
	receive_data(client->serverSocket, &controllerOne->usbPort, sizeof(int));
	swapbytes(&controllerOne->usbPort, sizeof(int));
	receive_data(client->serverSocket, &controllerOne->normalTs, sizeof(double));
	swapbytes(&controllerOne->normalTs, sizeof(double));
	receive_data(client->serverSocket, &controllerOne->setpoint, sizeof(double));
	swapbytes(&controllerOne->setpoint, sizeof(double));
	receive_data(client->serverSocket, &controllerOne->kp, sizeof(double));
	swapbytes(&controllerOne->kp, sizeof(double));
	receive_data(client->serverSocket, &controllerOne->ki, sizeof(double));
	swapbytes(&controllerOne->ki, sizeof(double));
	receive_data(client->serverSocket, &controllerOne->kd, sizeof(double));
	swapbytes(&controllerOne->kd, sizeof(double));

	// Controller Two
	receive_data(client->serverSocket, &controllerTwo->normalTs, sizeof(double));
	swapbytes(&controllerTwo->normalTs, sizeof(double));
	receive_data(client->serverSocket, &controllerTwo->setpoint, sizeof(double));
	swapbytes(&controllerTwo->setpoint, sizeof(double));
	receive_data(client->serverSocket, &controllerTwo->kp, sizeof(double));
	swapbytes(&controllerTwo->kp, sizeof(double));
	receive_data(client->serverSocket, &controllerTwo->ki, sizeof(double));
	swapbytes(&controllerTwo->ki, sizeof(double));
	receive_data(client->serverSocket, &controllerTwo->kd, sizeof(double));
	swapbytes(&controllerTwo->kd, sizeof(double));
	controllerTwo->usbPort = controllerOne->usbPort;
}


process_time_t GetTimeStruct(double Ts)
{
	process_time_t time;
	double seconds, nanoseconds;
	nanoseconds  = modf(Ts, &seconds) * 1000000000UL; // separate seconds and nanoseconds from incomming time value (given in seconds)
	time.tv_sec  = seconds;
	time.tv_nsec = nanoseconds;
	return time;
}


void ShowParameters(control_data_t *controller)
{
	printf("usbPort = %d \n",controller->usbPort);
	printf("setpoint = %f \n",controller->setpoint);
	printf("Kp = %f\n",controller->kp);
	printf("Ki = %f\n",controller->ki);
	printf("Kd = %f\n",controller->kd);
	printf("Ts = %f\n",controller->normalTs);
}





























/*

void get_control_signal(process_data_t *process_info)
{
	if(process_info->step_time <= process_info->current_time)
		process_info->control_signal = process_info->setpoint;
	else
		process_info->control_signal = 0.0;
	process_info->control_signal_swap = process_info->control_signal;
	swapbytes(&process_info->control_signal_swap,sizeof(double));
}

time_process_t get_time_struct(double Ts)
{
	time_process_t time;
	double seconds, nanoseconds;
	nanoseconds  = modf(Ts, &seconds) * 1000000000UL; // separate seconds and nanoseconds from incomming time value (given in seconds)
	time.tv_sec  = seconds;
	time.tv_nsec = nanoseconds;
	return time;
}

*/
