/*
 * consola.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "planificacion.h"
#include "sincronizacion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int flag;

void *menu(void *ptr);
void listar(char* rec);
void bloquear(char* clave, int id);
void desbloquear(char* clave);
void pausar_despausar_consola();
void mostrar_esi_en_pantalla(void* esi);

#endif /* CONSOLA_H_ */