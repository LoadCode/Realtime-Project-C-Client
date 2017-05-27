
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



int main()
{
	ControllerData_t processTemp, processFlow;
	int server_sock,client_sock;

    /*Mutex para el acceso a la pantalla*/
    omp_lock_t mtx_temp,mtx_flow,mtx_duqx;

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
	processTemp.ki = 0.00122;
	processTemp.kd = 0.005;
	processFlow.kp = 0.2298;
    processFlow.ki = 0.03396;
	processFlow.kd = 0.0;
	processTemp.normalTs = 0.1;
	processFlow.normalTs = 0.1;
	processTemp.setpoint = 2.1;
	processFlow.setpoint = 3.3;


	
    /*Inicializa mutex*/
	omp_init_lock(&mtx_temp);
    omp_init_lock(&mtx_flow);

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
    dUQx_Init(BOARD_USB);
    dUQX_SetResolution(processTemp.resolution);
    dUQx_CalibrateAnalog(&processTemp.refVolt);
	processFlow.refVolt = processTemp.refVolt;
    
	printf2("Creating threads...\n");
	
	/*Crear servidor y esperar conexión del cliente*/
	//server_create(SERVER_PORT, &server_sock);

	//client_wait(server_sock,&client_sock);
	
#pragma omp parallel default(none) shared(processTemp, processFlow, mtx_temp,mtx_flow,mtx_duqx) firstprivate(server_sock,client_sock) private(set,timer_data,timer_id,sig_num,event)
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
					GetControlSignal(&processTemp);

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
					GetControlSignal(&processFlow);

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
				//if(!server_create(SERVER_PORT, &server_sock))
				//if(!client_wait(server_sock,&client_sock))
                while(1)
                {

					// Recibir banderas
					
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
					/*swapbytes(aux_set,2*sizeof(double));
                    send_data(client_sock,aux_set,2*sizeof(double));
					
                    swapbytes(aux_v,2*sizeof(double));
                    send_data(client_sock,aux_v,2*sizeof(double));
					
					swapbytes(aux_vin,2*sizeof(double));
                    send_data(client_sock,aux_vin,2*sizeof(double));
					*/			

                }

            }	
         
        }


    }

    return(0);
}
