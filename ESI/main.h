/*
 * main.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <commons/log.h>
#include "../syntax-commons/my_socket.h"
#include "../syntax-commons/serializador.h"
#include "../syntax-commons/protocol.h"
#include "config_esi.h"
//#include "../syntax-commons/protocol.h"
#include "../syntax-commons/conexiones.h"
#include <parsi/parser.h>


#define CANT_ARGUMENTOS_MAIN 3
#define LOG_LEVEL LOG_LEVEL_TRACE

t_log* logger;

paquete *pkg_esi;

typedef struct
{
int keyword;
char *clave;
char *valor;
}t_operacion;

t_operacion *n_operacion;

config configuracion; //no le pongan otro nombre, porque despues limpiar_configuracion() se va a encargar de borrarla


#endif /* MAIN_H_ */