#ifndef DUQX_H_
#define DUQX_H_

#include <stdint.h>

/*Inicia dUQx: retorna 0 si la inicialización salió bien, 1 en caso contrario*/
uint8_t dUQx_Init(int);

void dUQx_End(void);

/*Verifica si dUQx est� conectada*/
void dUQx_IsReady(void);

/*Configura el puerto digital*/
void dUQx_ConfigureDigital(uint16_t );

/*Escribe el puerto digital*/
void dUQx_WriteDigital(uint16_t );

/*Lee el puerto digital*/
void dUQx_ReadDigital(uint16_t *);

/*Escribe el canal an�logo*/
void dUQx_WriteAnalog(double,double);

/*Lee un canal an�logo*/
void dUQx_ReadAnalogSingle(uint8_t ch,double vref,double * v);

uint8_t dUQx_ReadAnalogBuffer(uint8_t ch,double vref,double
** v,uint8_t n);

/*Calibra el controlador, retorna el voltaje de referencia estimado*/
void dUQx_CalibrateAnalog(double *);


/*Selecciona entre la resoluci�n de 10 bits (r=1) y 8 bits (r=0)*/
/*Por defecto dUQx opera a una resoluci�n de 10 bits*/
void dUQX_SetResolution(uint8_t r);


/*Preescalador del reloj del ADC*/
void dUQx_ADCPreescalerSet(uint8_t ps);


/*Habilita (e=1) o inhabilita (e=0) el ADC*/
void dUQx_ADCEnabledSet(uint8_t e);


#endif /*DUQX_H_*/
