#ifndef _UTILS_H_
#define _UTILS_H_
#include <time.h>
#include <uqeasysocket.h>


#define IP_SERVER   "127.0.0.1"
#define MAIN_PORT   34869
#define PORT_CLIENT 34869
#define REQUEST_VAL 45862
#define REQUEST_YES 2958
#define MAX_OUTPUT 5 //5 volts
#define MIN_OUTPUT 0 //0 volts
#define OPENLOOP_SIGNAL    54378
#define CLOSEDLOOP_SIGNAL  45876

typedef struct timespec process_time_t;
typedef enum{false, true} bool;

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
    int loopType;
    double currentTime;
    double normalTs; //sample time in seconds
    double currentTimeSwap;
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
process_time_t GetTimeStruct(double Ts);
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
