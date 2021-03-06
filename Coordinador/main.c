/*
 * main.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include "main.h"

#include <stdbool.h>
#include <stdlib.h>

#include "../syntax-commons/my_socket.h"
#include "config_coordinador.h"
#include "error.h"
#include "log_operaciones.h"
#include "servidor.h"
#include "sincronizacion.h"

int main(int argc, char **argv) { //aca recibiriamos la ruta del archivo de configuracion como parametro
	logger = log_create("coordinador.log", "Coordinador", true, LOG_LEVEL);
	if(argc != 2){
		log_error(logger, "Cantidad incorrecta de parámetros");
		log_destroy(logger);
		exit(EXIT_FAILURE);
	}
	configurar(argv[1]);
	log_trace(logger, "Coordinador correctamente configurado");
	crear_log_operaciones();
	lista_instancias_disponibles = list_create();
	lista_instancias_inactivas = list_create();
	inicializar_semaforos();
	int local_socket = crear_socket_escucha(configuracion.puerto, BACKLOG);

	log_info(logger, "Escuchando en puerto: %s", configuracion.puerto);

	if (pthread_create(&thread_listener, NULL,
			(void*) esperar_nuevas_conexiones, &local_socket))
		exit_error_with_msg("Error creando el hilo del servidor escucha\n");

	if (pthread_join(thread_listener, NULL))
		exit_error_with_msg("Error al joinear thread del servidor escucha");

	morir_liberando_recursos(EXIT_SUCCESS);
}

