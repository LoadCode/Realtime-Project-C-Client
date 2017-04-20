#ifndef _UTILS_H_
#define _UTILS_H_
#include <time.h>
#include <uqeasysocket.h>

typedef struct timespec process_time_t;

typedef struct
{
	/*Board related info*/
	int usbPort;
    int analogInput;
    int analogOutput;
    int resolution;

    /*Controller related info*/
    double setpoint;
    double kp;
    double ki;
    double kd;
    double currentTime;
    double normalTs; //sample time in seconds
    process_time_t sampleTime;
} control_data_t;


typedef struct
{
	char ip[15];
	int serverSocket;
    int clientSocket;
    int port;
    int requestValue;
    int serverAnswer;
} client_data_t;

void GettingParameters(client_data_t *client, control_data_t *controllerOne, control_data_t *controllerTwo);

void ShowParameters(control_data_t *controller);

#endif //_UTILS_H_

























/*
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
	double current_time;
	double setpoint;
	double step_time;
	double control_signal;
	double control_signal_swap;
	time_process_t sample_time;
} process_data_t;

void get_control_signal(process_data_t *process_info);
time_process_t get_time_struct(double Ts);
*/
