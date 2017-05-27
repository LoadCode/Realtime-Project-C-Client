#ifndef _UTILS_H_
#define _UTILS_H_
#include <time.h>
#define MAX_OUTPUT 5 //5 volts
#define MIN_OUTPUT 0 //0 volts

typedef struct timespec process_time_t;

typedef struct
{
	/*Board related info*/
	int usbPort;
    int analogInput;
    int analogOutput;
    int resolution;
    int finishFlag;
    double refVolt;

    /*Controller related info*/
    double setpoint;
    double processOutput;
    double swapProcessOutput;
    double controllerOutput;
    double swapControlOutput;
    double error;
    double iTerm; // integral term
    double lastProcessOutput;
    double kp;
    double ki;
    double kd;
    double currentTime;
    double normalTs; //sample time in seconds
    double currentTimeSwap;
    process_time_t sampleTime;
} ControllerData_t;


typedef struct
{
	int serverSocket;
    int clientSocket;
    int port;
} client_data_t;


void GetControlSignal(ControllerData_t * controlData);
process_time_t GetTimeStruct(double Ts);
void ShowParameters(ControllerData_t *controller);

#endif //_UTILS_H_
