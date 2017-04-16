/*dUQx 2.0 driver*/
/*Alexander López Parrado*/



#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <RS-232/rs232.h>




#include <requests.h>   /* custom request numbers */

/*Manejador del dispositivo*/
int handle;

/*Resolución, 1: 10 bits, 0: 8 bits*/
unsigned char res_mode=1;
 
/*Main communication function through serial port*/
 void serial_control_msg(int handle, uint8_t *buffer){

/*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 3);

 /*Reads response from dUQx*/
RS232_PollComport(handle, buffer, 2);




 }



/*Inicia dUQx*/
uint8_t dUQx_Init(int cport_nr) {

int    bdrate=500000,ret;

  char mode[]={'8','N','1',0};

handle=cport_nr;

ret=RS232_OpenComport(cport_nr, bdrate, mode);



    //Waits for bootloader execution
    #ifdef _WIN32
    Sleep(3000);
#else
    sleep(3);  /* sleep for 1 Second */
#endif


    //10-bit mode by default
    res_mode=1;

return(ret);

}


void dUQx_End(void){

  RS232_CloseComport(handle);
    }

void dUQx_ConfigureDigital(uint16_t ddr){

uint8_t buffer[3];

buffer[0]=DUQX_DIGITAL_CONFIGURE;
buffer[1]=ddr&0xff;
buffer[2]=(ddr>>8)&0x3;

/*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 3);







}


void dUQx_WriteDigital(uint16_t port){

uint8_t buffer[3];

/*Escribir digital*/

buffer[0]=DUQX_DIGITAL_WRITE;
buffer[1]=port&0xff;
buffer[2]=(port>>8)&0x3;

/*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 3);


}


void dUQx_ReadDigital(uint16_t *pin)
{

uint8_t buffer[2];

buffer[0]=DUQX_DIGITAL_READ;

/*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 1);
#if defined(__linux__) || defined(__FreeBSD__)
set_blocking (handle, 2);
#endif // defined
 /*Reads response from dUQx*/
RS232_PollComport(handle, buffer, 2);


*pin=((uint16_t)(buffer[1])<<8|(uint16_t)(buffer[0]))&0x3ff;

}


void dUQx_WriteAnalog(double v,double vref){

    uint16_t v_i;
    uint8_t buffer[3];







	if(v>vref) v=vref;
	if(v<0) v=0.0;


	v_i= (uint16_t)(1023.0*(double)(v)/vref);

	 buffer[0]=DUQX_DAC_WRITE_SINGLE;
buffer[1]=v_i&0xff;
buffer[2]=(v_i>>8)&0x3;

/*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 3);



}



void dUQx_ReadAnalogSingle(uint8_t ch,double vref,double * v){


    uint16_t v_i;
    uint8_t buffer[2];

    buffer[0]=DUQX_ADC_READ_SINGLE;
    buffer[1]=ch&0x0f;

    /*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 2);





    if(res_mode){

    #if defined(__linux__) || defined(__FreeBSD__)
set_blocking (handle, 2);
#endif // defined

     /*Reads response from dUQx*/
RS232_PollComport(handle, buffer, 2);


             v_i=((uint16_t)(buffer[1])<<8|(uint16_t)(buffer[0]))&0x3ff;

         *v=(((double)(v_i)*vref)/1023.0);

    }
     else{
     #if defined(__linux__) || defined(__FreeBSD__)
set_blocking (handle, 1);
#endif // defined
      RS232_PollComport(handle, buffer, 1);
             v_i=((uint16_t)(buffer[0]))&0xff;

         *v=(((double)(v_i)*vref)/255.0);

     }



}




uint8_t dUQx_ReadAnalogBuffer(uint8_t ch,double vref,double
** v,uint8_t n){

 uint16_t v_i,i,len;
    uint8_t *buffer;

    double *p_v;

    uint8_t buffer2[4];



/*Escribir digital*/

buffer2[0]=DUQX_ADC_READ_BUFFER;
buffer2[1]=ch;
buffer2[2]=n;
buffer2[3]=(n>>8);

RS232_SendBuf(handle, buffer2, 4);







    if(res_mode){
 if(n>127)
        n=127;

          p_v=(double*)malloc(n*sizeof(double));

    *v=p_v;

            buffer=(uint8_t*)malloc(2*n*sizeof(uint8_t));

#if defined(__linux__) || defined(__FreeBSD__)
set_blocking (handle, 2*n);
#endif // defined
            len=RS232_PollComport(handle, buffer, 2*n);



     len=len>>1;

for(i=0;i<len;i++){
 v_i=(((uint16_t)(buffer[2*i+1])<<8)|((uint16_t)(buffer[2*i])))&0x3ff;


         p_v[i]=(((double)(v_i)*vref)/1023.0);
}



    }
     else{
 if(n>255)
        n=255;

          p_v=(double*)malloc(n*sizeof(double));

    *v=p_v;

             buffer=(uint8_t*)malloc(n*sizeof(uint8_t));

             #if defined(__linux__) || defined(__FreeBSD__)
set_blocking (handle, n);
#endif // defined
         len=RS232_PollComport(handle, buffer, n);
         for(i=0;i<len;i++)
            p_v[i]=(((double)(buffer[i])*vref)/255.0);



     }


free(buffer);

     return len;



}


void dUQx_CalibrateAnalog(double *vref){

double aux;

	/*Se lee el voltaje del regulador*/
	dUQx_ReadAnalogSingle(14,1.0,&aux);
	dUQx_ReadAnalogSingle(14,1.0,&aux);


	/*Se estima el voltaje de referencia*/
	aux=1.1/aux;

	*vref=aux;

}


void dUQX_SetResolution(uint8_t r){
    uint8_t buffer[2];

    res_mode=r;

    buffer[0]=DUQX_ADC_RESOLUTION_SET;
buffer[1]=r;


   /*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 2);



}






void dUQx_ADCPreescalerSet(uint8_t ps)
{
    uint8_t buffer[2];
           /*Leer digital*/
    buffer[0] =DUQX_ADC_PRESCALER_SET;
    buffer[1]=ps;

       /*Writes commands to dUQx*/
RS232_SendBuf(handle, buffer, 2);

}



void dUQx_ADCEnabledSet(uint8_t e)
{
    uint8_t data;
           /*Leer digital*/

      data =DUQX_ADC_ENABLED_SET;

   /*Writes commands to dUQx*/
RS232_SendBuf(handle, &data, 1);



}





