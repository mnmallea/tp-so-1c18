/*
 * main.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include "main.h"


int main(int argc, char **argv){//aca recibiriamos la ruta del archivo de configuracion como parametro


	lista_esis_listos = list_create();
	esi_corriendo=(t_esi *)malloc(sizeof(t_esi));
	lista_esis_finalizados = list_create();
	dic_esis_bloqueados = dictionary_create();
	dic_esi_recurso_bloq = dictionary_create();

	/*Config*/
	logger = log_create("planificador.log","Planificador",true,LOG_LEVEL);
	logger -> is_active_console=0;
	configuracion = configurar(argv[1]);
	/*Creacion de hilos*/
	pthread_t selector_planificador;
	pthread_t consola_planificador;
	pthread_t planificador;

	const char *message0 = "Inicializacion el selector";
	if(pthread_create(&selector_planificador, NULL, listener, (void*) message0)) {
		log_error(logger, "Cantidad incorrecta de parametros");
			exit(EXIT_FAILURE);
	}
	const char *message1 = "Inicializacion de la consola";
	if(pthread_create(&consola_planificador, NULL, menu, (void*) message1)) {
		log_error(logger, "Error creando el hilo de la consola\n");
		 exit(EXIT_FAILURE);
	}

	const char *message2 = "Inicializacion del planificador";
		if(pthread_create(&planificador, NULL, menu, (void*) message2)) {
			log_error(logger, "Error creando el hilo del planificador\n");
			 exit(EXIT_FAILURE);
		}

	/*Join threads*/
	if(pthread_join(selector_planificador, NULL)) {
		log_error(logger, "Error al joinear el hilo del selector\n");
			exit(EXIT_FAILURE);
	}
	if(pthread_join(consola_planificador, NULL)) {
		log_error(logger, "Error al joinear el hilo de la consola\n");
		 exit(EXIT_FAILURE);
	}
	if(pthread_join(planificador, NULL)) {
			log_error(logger, "Error al joinear el hilo del planificador\n");
			 exit(EXIT_FAILURE);
		}
	return 0;
}




