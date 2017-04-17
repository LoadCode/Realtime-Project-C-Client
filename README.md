# Realtime-Project-C-Client
Este repositorio contiene la implementación de la aplicación en C que se encarga de realizar el muestreo y control del proceso (Planta de flujo y temperatura), para dar cumplimiento al proyecto de la electiva de Tiempo Real.

## Compilación:
Para compilar este cliente en C solo debe ejecutar en la terminal el comando `make `, 
en seguida aparecerá en su directorio (junto a los archivos fuente) el ejecutable
de nombre `main` y que puede ser puesto en marcha escribiendo `./main`.

## Notas Importantes:
Tenga en cuenta que el funcionamiento tanto de servidor como de cliente se presenta en el siguiente orden:

* Ejecute el servidor Java en una terminal (solo ejecute, no inicie la adquisición aún)
* Ejecute el cliente en C en otra terminal (./main y verifique que ningún error se haya generado en ninguna de las dos terminales)
* Puede dar inicio a la adquisición de datos presionando el botón adecuado en la interfaz gráfica del servidor Java



