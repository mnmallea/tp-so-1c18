/*
 * main.c
 *
 *  Created on: 29 abr. 2018
 *      Author: utnso
 */
#include "main.h"

int main(int argc, char** argv) {
	logger = log_create("instancia.log", "Instancia", true, LOG_LEVEL);
	configuracion = configurar(argv[1]);
	int socketCoordinador = conectarse_a_coordinador(
			configuracion.ip_coordinador, configuracion.puerto_coordinador,
			INSTANCIA);
	configurarAlmacenamiento(socketCoordinador);
	log_trace(logger,
			"Se va a inicializar el almacenamiento con: cant_entradas = %d y tamaño = %d",
			cfgAlmacenamiento.totalEntradas, cfgAlmacenamiento.tamanioEntrada);
	inicializarAlmacenamiento(cfgAlmacenamiento.totalEntradas,
			cfgAlmacenamiento.tamanioEntrada);
	crearTablaEntradas();
	iniciarDumper(configuracion.punto_montaje);
	int escucha = 1;
	pthread_t dumper;
	//tengo que hacer pthread join ?
    if (pthread_create(&dumper,NULL,dumpearADisco,NULL)) {
			log_error(logger, "Error creando el hilo del dumper\n");
			exit(EXIT_FAILURE);
	}
	t_list* posiblesAReemplazar=NULL;
	while (escucha) {
		int resultado;
		switch (recibir_cod_operacion(socketCoordinador)) {
		case OP_SET:
			resultado = SET(socketCoordinador,posiblesAReemplazar);
			if (resultado >= 0) {
				enviar_cod_operacion(socketCoordinador, EXITO);
				mandar_mensaje(socketCoordinador,obtenerEntradasTotales()- entradasLibres);
			} else {

				enviar_cod_operacion(socketCoordinador, ERROR);
			}
			break;
		case OP_STORE:
			;
			char* clave;
			resultado = recibir_operacion_unaria(socketCoordinador, &clave);
			if (resultado < 0) {
				log_error(logger, "Error al recibir operacion ");
				close(socketCoordinador);
				exit(EXIT_FAILURE);
			}
			log_trace(logger, "Store %s", clave);
			resultado = STORE(clave);
			if (resultado >= 0) {
				enviar_cod_operacion(socketCoordinador, EXITO);
				mandar_mensaje(socketCoordinador,obtenerEntradasTotales()- entradasLibres);
			} else {
				enviar_cod_operacion(socketCoordinador, ERROR);
			}
			break;

		case MATAR_INSTANCIA:
			log_info(logger, "la instancia se esta desconectando");
			eliminarAlmacenamiento();
			//todo: destruirTE();
			free(posiblesAReemplazar);
			close(socketCoordinador);
			escucha = 0;

			break;

		/*case RELEVANTAR_INSTANCIA:
			log_info(logger, "La instancia se esta relevantando.....");
			int* cantidadClaves=NULL;
			recibirPaquete(socketCoordinador,cantidadClaves, sizeof(int));
			int cantClaves= *cantidadClaves;
			free(cantidadClaves);
			for(int i=0;i<cantClaves;i++){
				void* clave;
				try_recibirPaqueteVariable(socketCoordinador, &clave);
				if(dictionary_has_key(dumper->fd, clave)){
					if(fopen(clave,"r")<0){
						log_info(logger," la instancia no encuentra el archivo de la clave %d",clave);
					}else{
						void* valor=NULL;/*hago el malloc o lo hace el fread*/
						/*fread (valor,sizeof(clave), 1,clave);
						SET(socketCoordinador,posiblesAReemplazar);
						free(valor);
						fclose(clave);
					}
				}
				free(clave);
			}
			break;*/
		case INSTANCIA_COMPACTAR:
			log_info(logger, "Estoy compactando ...");
			enviar_cod_operacion(socketCoordinador, EXITO);
			//si falla deberia contestarle ERROR
			break;
		default: 
			log_info(logger, "no se pudo interpretar el mensaje");
			eliminarAlmacenamiento();
			//TODO	destruirTE();
			free(posiblesAReemplazar);
			close(socketCoordinador);
			escucha=0;
		

		}
	}

	limpiar_configuracion();
	log_destroy(logger);
	exit(0);
}
