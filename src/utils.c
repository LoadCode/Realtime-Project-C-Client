#include <utils.h>
#include <stdio.h>
#include <math.h>


void GettingParameters(client_data_t *client, control_data_t *controllerOne, control_data_t *controllerTwo)
{
	/*Orden del paso de parámetros desde el servidor
	 * CONTROLLER ONE
	 * usbPort
	 *
	 * Signal (Open-loop or Closed-loop) of process one
	 * Ts
	 * setpoint
	 * kp
	 * ki
	 * kd
	 *
	 * CONTROLLER TWO
	 *
	 * Signal (Open-loop or Closed-loop) of process two
	 * Ts
	 * setpoint
	 * kp
	 * ki
	 * kd
	 * */

	// Gets the usb port of dUQx
	receive_data(client->serverSocket, &controllerOne->usbPort, sizeof(int));
	swapbytes(&controllerOne->usbPort, sizeof(int));
	controllerTwo->usbPort = controllerOne->usbPort;

	// Gets loop-type signal for controller one
	receive_data(client->serverSocket, &controllerOne->loopType, sizeof(int));
	swapbytes(&controllerOne->loopType, sizeof(int));

	// Controller One parameters
	if(controllerOne->loopType == OPENLOOP_SIGNAL)
	{
		receive_data(client->serverSocket, &controllerOne->normalTs, sizeof(double));
		swapbytes(&controllerOne->normalTs, sizeof(double));
		receive_data(client->serverSocket, &controllerOne->setpoint, sizeof(double));
		swapbytes(&controllerOne->setpoint, sizeof(double));
		controllerOne->kp = 0;
		controllerOne->ki = 0;
		controllerOne->kd = 0;
	}
	else //closed is assumed
	{
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

		// Discretize
		controllerOne->ki *= controllerOne->normalTs;
		controllerOne->kd /= controllerOne->normalTs;
	}


	// Gets loop-type signal for controller one
	receive_data(client->serverSocket, &controllerTwo->loopType, sizeof(int));
	swapbytes(&controllerTwo->loopType, sizeof(int));

	// Controller Two parameters
	if(controllerTwo->loopType == OPENLOOP_SIGNAL)
	{
		receive_data(client->serverSocket, &controllerTwo->normalTs, sizeof(double));
		swapbytes(&controllerTwo->normalTs, sizeof(double));
		receive_data(client->serverSocket, &controllerTwo->setpoint, sizeof(double));
		swapbytes(&controllerTwo->setpoint, sizeof(double));
		controllerTwo->kp = 0;
		controllerTwo->ki = 0;
		controllerTwo->kd = 0;
	}
	else // closedloop is assumed
	{
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

		// Discretize
		controllerTwo->ki *= controllerTwo->normalTs;
		controllerTwo->kd /= controllerTwo->normalTs;
	}
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
