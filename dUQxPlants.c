
#include <omp.h>
#include <stdio.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>
#include <dUQx.h>
#include <uqeasysocket.h>
#include "utils.h"

#define BOARD_USB 16
#define SERVER_PORT 9090


/*tipo de dato para los temporizadores*/
typedef struct itimerspec mytimer_t;



int main(int nargs, char* argsv[])
{
	ControllerData_t processTemp, processFlow;
	int server_sock,client_sock;
	int board_usb;
	int finishCommFlag = 0, newDataFlag = 1;
	double aux_setpoint = 0;
    /*Mutex para el acceso a la pantalla*/
    omp_lock_t mtx_temp,mtx_flow,mtx_duqx, mtx_spn_temp, mtx_spn_flow;

    /*Conjunto de señales que manejará*/
    sigset_t set;
    /*Identificador del temporizador*/
    timer_t timer_id;
    /*Número de la señal que generará el temporizador*/
    int sig_num;
    /*Estructura con la configuración del evento del temporizador*/
    struct sigevent event;
    /*Estructura con la información temporal del temporizador*/
    mytimer_t timer_data;

	if(nargs != 2)
	{
		printf("Please, set the port number of the board...\n");
		return 0;
	}
	else
	{
		board_usb = atoi(argsv[1]);
	}
	
    // Inicializaciones
	processTemp.finishFlag   = 0;
	processFlow.finishFlag   = 0;
	processTemp.resolution   = 1; // Resolución de 10 bits para el ADC
	processFlow.resolution   = 1; // Resolución de 10 bits para el ADC
	processTemp.analogInput  = ADC_CHANNEL_TEMP; // Arduino A0 pin
	processFlow.analogInput  = ADC_CHANNEL_FLOW; // Arduino A1 pin
	processTemp.analogOutput = DAC_CHANNEL_TEMP; // First controller's output in Arduino pwm pin 9
	processFlow.analogOutput = DAC_CHANNEL_FLOW;// First controller's output in Arduino pwm pin 10
	processTemp.iTerm        = 0.0;
	processFlow.iTerm        = 0.0;
	processTemp.lastProcessOutput = 0;
	processFlow.lastProcessOutput = 0;
	
	// Valores iniciales de Ki, Kp, Kd
	processTemp.kp = 1.101; //0.8021;
	processTemp.ki = 0.00322;
	processTemp.kd = 0.007;
	processFlow.kp = 0.2298;
    processFlow.ki = 0.04396;
	processFlow.kd = 0.0;
	processTemp.currentTime = 0.0;
	processFlow.currentTime = 0.0;
	processTemp.normalTs = 0.1;
	processFlow.normalTs = 0.1;
	processTemp.setpoint = 0.0;
	processFlow.setpoint = 0.0;


	
    /*Inicializa mutex*/
	omp_init_lock(&mtx_temp);
    omp_init_lock(&mtx_flow);
	omp_init_lock(&mtx_spn_flow);
	omp_init_lock(&mtx_spn_temp);
    omp_set_lock(&mtx_temp);
    omp_set_lock(&mtx_flow);

    omp_init_lock(&mtx_duqx);

    /*Se configura la máscara para el main, los hilos hijos la heredarán*/
    sigemptyset (&set);
    sigaddset (&set, SIGRTMIN+0);
    sigaddset (&set, SIGRTMIN+1);
    /*Se bloquean ambas señales ya que no habra un callback en el manejador,
    * estas serán manejadas por sigwait()*/
    pthread_sigmask (SIG_BLOCK, &set, NULL) ;

	/*Inicia conio*/
    consoleinit();

    /*Se fija la cantidad de hilos a dos */
    omp_set_num_threads(3);

	/*Inicializa dUQx*/
    if( dUQx_Init(board_usb))
	{
		printf("\rLa tarjeta esta desconectada o el puerto no corresponde\n\r");
		return 0;
	}	
    dUQX_SetResolution(processTemp.resolution);
    dUQx_CalibrateAnalog(&processTemp.refVolt);
	processFlow.refVolt = processTemp.refVolt;
    
	printf2("Creating threads...\n");
	
	/*Crear servidor y esperar conexión del cliente*/
	//server_create(SERVER_PORT, &server_sock);

	//client_wait(server_sock,&client_sock);
	
#pragma omp parallel default(none) shared(processTemp, processFlow, mtx_temp,mtx_flow,mtx_duqx, mtx_spn_temp, mtx_spn_flow) firstprivate(server_sock,client_sock) private(set,timer_data,timer_id,sig_num,event,aux_setpoint) firstprivate(finishCommFlag, newDataFlag)
    {

        #pragma omp sections
        {
			// Control para temperatura
            #pragma omp section
            {   
			    /*Señal que será generada por el temporizador*/
                sig_num = SIGRTMIN+0;
				
                /*Se adiciona la señal*/
                sigemptyset (&set);
                sigaddset (&set, sig_num);
				
                /*Se configura el evento del temporizador*/
                event.sigev_notify = SIGEV_SIGNAL;
                event.sigev_signo = sig_num;
				
                /*Se crea el temporizador*/
                timer_create (CLOCK_REALTIME, &event, &timer_id);
				
                /*Se configura el periodo del temporizador*/
				processTemp.sampleTime = GetTimeStruct(processTemp.normalTs);
				timer_data.it_interval = processTemp.sampleTime;
				timer_data.it_value    = processTemp.sampleTime;
				
                /*Se inicia el temporizador*/
                timer_settime (timer_id, 0, &timer_data, NULL);
				
                while (1)
                {

                    /*Se espera la señal del temporizador*/
                    sigwait(&set,&sig_num);
                    /*Código del hilo*/

					// Lectura de la tarjeta
                    omp_set_lock(&mtx_duqx);
                    dUQx_ReadAnalogSingle(processTemp.analogInput, processTemp.refVolt, &processTemp.processOutput);  
                    omp_unset_lock(&mtx_duqx);
					
                    // Cálculo del PID
					omp_set_lock(&mtx_spn_temp);
					GetControlSignal(&processTemp);
					processTemp.swapControlOutput = processTemp.controllerOutput;
					omp_unset_lock(&mtx_spn_temp);
					
					// Escritura en la tarjeta
					omp_set_lock(&mtx_duqx);
                    dUQx_WriteAnalog(processTemp.controllerOutput,processTemp.refVolt,DAC_CHANNEL_TEMP);
                    omp_unset_lock(&mtx_duqx);
					
                    /*Se informa al hilo de comunicación*/		
                    omp_unset_lock(&mtx_temp);
                }
            }

			// Control para flujo
            #pragma omp section
            {
                /*Señal que será generada por el temporizador*/
                sig_num = SIGRTMIN+1;

                /*Se adiciona la señal*/
                sigemptyset (&set);
                sigaddset (&set, sig_num);
                /*Se configura el evento del temporizador*/
                event.sigev_notify = SIGEV_SIGNAL;
                event.sigev_signo = sig_num;

                /*Se crea el temporizador*/
                timer_create (CLOCK_REALTIME, &event, &timer_id);

                /*Se configura el periodo del temporizador*/
                processFlow.sampleTime = GetTimeStruct(processFlow.normalTs);
				timer_data.it_interval = processFlow.sampleTime;
				timer_data.it_value    = processFlow.sampleTime;
				
                /*Se inicia el temporizador*/
                timer_settime (timer_id, 0, &timer_data, NULL);

                while (1)
                {

                    /*Se espera la señal del temporizador*/
                    sigwait(&set,&sig_num);
                    /*Código del hilo*/
		            omp_set_lock(&mtx_duqx);
                    dUQx_ReadAnalogSingle(processFlow.analogInput, processFlow.refVolt, &processFlow.processOutput);
					omp_unset_lock(&mtx_duqx);

					// Cálculo del PID
					omp_set_lock(&mtx_spn_flow);
					GetControlSignal(&processFlow);
					processFlow.swapControlOutput = processFlow.controllerOutput;
					omp_unset_lock(&mtx_spn_flow);
					
					omp_set_lock(&mtx_duqx);
					dUQx_WriteAnalog(processFlow.controllerOutput,processFlow.refVolt, processFlow.analogOutput);
                    omp_unset_lock(&mtx_duqx);
					
                    /*Se informa al hilo de comunicación*/ 	
                    omp_unset_lock(&mtx_flow);
                }
            }
	
			/*****************************************************************
             Hilo de comunicación (servidor)
             ****************************************************************/	
            #pragma omp section
            {

                /*Crear servidor y esperar conexión del cliente*/
				if(!server_create(SERVER_PORT, &server_sock))
				{
					//printf("Creo el servidor\n");
					if(!client_wait(server_sock,&client_sock))
					{
						//printf("El cliente se conecto\n");
						while(1)
						{
							// Recibir banderas
							receive_data(client_sock, &finishCommFlag, sizeof(int));
							receive_data(client_sock, &newDataFlag, sizeof(int));
							swapbytes(&finishCommFlag, sizeof(int));
							if(finishCommFlag == 1)
								break;
							swapbytes(&newDataFlag, sizeof(int));
							if(newDataFlag == 1)
							{
								// Recibir nuevos parámetros de setpoint
								receive_data(client_sock, &aux_setpoint,sizeof(double));
								swapbytes(&aux_setpoint,sizeof(double));
								omp_set_lock(&mtx_spn_temp);
								processTemp.setpoint = aux_setpoint;
								omp_unset_lock(&mtx_spn_temp);
								receive_data(client_sock, &aux_setpoint,sizeof(double));
								swapbytes(&aux_setpoint, sizeof(double));
								omp_set_lock(&mtx_spn_flow);
								processFlow.setpoint = aux_setpoint;
								omp_unset_lock(&mtx_spn_flow);
							}
							
							/*Esperar que el primer hilo lea el canal analógico 14*/
							omp_set_lock(&mtx_temp);
							
							gotoxy(0,1);
							printf2("Setpoint Temperatura: %lf v\n",processTemp.setpoint);
							gotoxy(0,2);
							printf2(" Salida de Temperatura: %lf v\n",processTemp.processOutput);
							gotoxy(0,3);
							printf2("Entrada de Temperatura: %lf v\n",processTemp.controllerOutput);
							
							
							/*Esperar que el segundo hilo lea el canal analógico 15*/	
							omp_set_lock(&mtx_flow);
                    
							gotoxy(0,5);
							printf2("     Setpoint de Flujo: %lf v\n", processFlow.setpoint);
							gotoxy(0,6);
							printf2("       Salida de Flujo: %lf v\n", processFlow.processOutput);
							gotoxy(0,7);
							printf2("      Entrada de Flujo: %lf v\n", processFlow.controllerOutput);
                    
							/*Enviar los voltajes al cliente*/

							processTemp.swapControlOutput = processTemp.controllerOutput;
							swapbytes(&processTemp.swapControlOutput, sizeof(double));
							send_data(client_sock,&processTemp.swapControlOutput,sizeof(double));
							
							processFlow.swapControlOutput = processFlow.controllerOutput;
							swapbytes(&processFlow.swapControlOutput, sizeof(double));
						    send_data(client_sock,&processFlow.swapControlOutput,sizeof(double));
							
							processTemp.swapProcessOutput = processTemp.processOutput;
							swapbytes(&processTemp.swapProcessOutput, sizeof(double));
							send_data(client_sock,&processTemp.swapProcessOutput,sizeof(double));

							processFlow.swapProcessOutput = processFlow.processOutput;
							swapbytes(&processFlow.swapProcessOutput, sizeof(double));
							send_data(client_sock,&processFlow.swapProcessOutput,sizeof(double));

							processTemp.currentTimeSwap   = processTemp.currentTime;
							swapbytes(&processTemp.currentTimeSwap,sizeof(double));
							send_data(client_sock,&processTemp.currentTimeSwap,sizeof(double));

						   	processTemp.currentTime += processTemp.normalTs;
							processFlow.currentTime += processFlow.normalTs;
						}
						close_socket(client_sock);
						close_socket(server_sock);
					}
				}
            }	
       }
    }

    return(0);
}
