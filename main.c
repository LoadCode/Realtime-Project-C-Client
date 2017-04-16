#include <math.h>
#include <stdlib.h>
#include <uqeasysocket.h>

#define IP_SERVER "192.168.0.20"
#define PORT 2222
#define REQUEST_VAL 45862
#define REQUEST_YES 2958
#define TOL 0.000001

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
	int req_answer, i = 1;
	int vector_length = 50;
	int rev_vec_len;
	double vector_vals[vector_length];
	double rev_vec_vals[vector_length];
	double norm_result = 0.0, norm_result_server;

	// creates client
	if(client_create(IP_SERVER, PORT, &client_info.server_socket))
	{
		printf("Error creando el cliente\n");
		return 0;
	}

	// try to connect with server
	swapbytes(&request, sizeof(int));
	do
	{
		printf("enviando solicitud\n");
		send_data(client_info.server_socket, &request, sizeof(int));
		//printf("recibiendo respuesta\n");
		receive_data(client_info.server_socket, &req_answer, sizeof(int));
		swapbytes(&req_answer, sizeof(int));
		printf("Respuesta request: %d\n",req_answer);
		printf("intento: %d\n",i);
		i++;
	}while(req_answer != REQUEST_YES);

	// send length of vector
	rev_vec_len = vector_length;
	swapbytes(&rev_vec_len, sizeof(int));
	send_data(client_info.server_socket, &rev_vec_len, sizeof(int));

	// generate random vector
	for(int i =0; i<vector_length; i++)
	{
		vector_vals[i] = (float)rand()/(float)(RAND_MAX/50) +i;
		rev_vec_vals[i] = vector_vals[i];
		swapbytes(&rev_vec_vals[i], sizeof(double));
	}

	// send vector elements
	for(int i =0; i < vector_length;  i++)
		send_data(client_info.server_socket, &rev_vec_vals[i], sizeof(double));

	// compute the vector norm
	//norm_result = norm(vector_vals, vector_length);
	for(int i = 0;i<vector_length; i++)
		norm_result += vector_vals[i] * vector_vals[i];
	norm_result = sqrt(norm_result);

	receive_data(client_info.server_socket, &norm_result_server, sizeof(double));
	swapbytes(&norm_result_server, sizeof(double));

	// compare answers
	if(abs(norm_result-norm_result_server) < TOL)
		printf("The result difference is less than %g\n",TOL);
	else
		printf("The result difference is greater than %g\n",TOL);

	// shows answer
	printf("Local Norm: %g\n",norm_result);
	printf("Server Norm: %g\n",norm_result_server);

	return 0;
}



double norm(double * array, int size)
{
	double sum = 0;
	for(int i = 0; i < size; i++)
		sum += array[i]*array[i];
	return sqrt(sum);
}

