#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <uqeasysocket.h>

#define IP_SERVER "192.168.0.20"
#define PORT 2222
#define REQUEST_VAL 45862
#define REQUEST_YES 2958

struct socket_info
{
	int server_socket;
	int client_socket;
};

typedef struct socket_info socket_info_t;


double norm(double * array, int size);


int main()
{
	socket_info_t client_info;
	int request = REQUEST_VAL;
	int req_answer;
	int finish = 0;
	double i = 0, i_swap = 0;
	int Ts = 5;
	
	

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
	printf("Respuesta request: %d\n",req_answer);
	if(req_answer != REQUEST_YES)
		return 0;

	receive_data(client_info.server_socket, &Ts, sizeof(int));
	swapbytes(&Ts, sizeof(int));
	printf("Ts = %d\n",Ts);
	do
	{
		swapbytes(&i_swap, sizeof(double));
		send_data(client_info.server_socket, &i_swap, sizeof(double));
		receive_data(client_info.server_socket, &finish, sizeof(int));
		swapbytes(&finish, sizeof(int));
		i++;
		i_swap = i;
		printf("contanto i = %f\n",i);
		sleep(Ts);
	}while(finish == 0);
	printf("Proceso de cliente terminado bien\n");
	close_socket(client_info.server_socket);

	return 0;
}
