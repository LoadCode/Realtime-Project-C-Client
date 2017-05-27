#include "utils.h"
#include <math.h>
#include <stdio.h>

process_time_t GetTimeStruct(double Ts)
{
	process_time_t time;
	double seconds, nanoseconds;
	nanoseconds  = modf(Ts, &seconds) * 1000000000UL; // separate seconds and nanoseconds from incomming time value (given in seconds)
	time.tv_sec  = seconds;
	time.tv_nsec = nanoseconds;
	return time;
}


void ShowParameters(ControllerData_t *controller)
{
	printf("usbPort = %d \n",controller->usbPort);
	printf("setpoint = %f \n",controller->setpoint);
	printf("Kp = %f\n",controller->kp);
	printf("Ki = %f\n",controller->ki);
	printf("Kd = %f\n",controller->kd);
	printf("Ts = %f\n",controller->normalTs);
}


void GetControlSignal(ControllerData_t * controlData)
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




