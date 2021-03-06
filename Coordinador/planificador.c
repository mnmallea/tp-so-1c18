/*
 * planificador.c
 *
 *  Created on: 7 jun. 2018
 *      Author: utnso
 */

#include "planificador.h"

#include <stdlib.h>
#include <string.h>

#include "../syntax-commons/my_socket.h"
#include "../syntax-commons/serializador.h"
#include "algoritmos_distribucion.h"
#include "error.h"
#include "instancia.h"

t_paquete* empaquetar_status_clave(char* clave);

void consulta_de_clave(char* clave, t_protocolo tipo_consulta) {
	t_paquete* paquete = paquete_crear();
	paquete_agregar(paquete, clave, strlen(clave) + 1);
	if (paquete_enviar_con_codigo(paquete, tipo_consulta, socket_planificador)
			< 0) {
		paquete_destruir(paquete);
		//hay que liberar varias cosas
		exit_error_with_msg("Error en la conexion con el planificador al realizar consulta de clave");
	}
	paquete_destruir(paquete);
}

void solicitar_clave(char* clave, t_esi* esi) {
	consulta_de_clave(clave, SOLICITUD_CLAVE);
	safe_send(socket_planificador, &esi->id, sizeof(esi->id));
}

void informar_liberacion_clave(char* clave) {
	consulta_de_clave(clave, DESBLOQUEO_CLAVE);
}

void esi_tiene_clave(char* clave, t_esi* esi) {
	consulta_de_clave(clave, ESI_TIENE_CLAVE);
	safe_send(socket_planificador, &esi->id, sizeof(esi->id));
}

void informar_instancia_caida(t_instancia* instancia) {
	t_protocolo msg = INSTANCIA_CAIDA_EXCEPTION;
	safe_send(socket_planificador, &msg, sizeof(msg));
}

void agregar_status_a_paquete(t_paquete* paquete, t_status_clave status) {
	paquete_agregar_sin_tamanio(paquete, &status, sizeof(status));
}

t_paquete* empaquetar_status_clave(char* clave) {
	t_paquete* paquete = paquete_crear();
	t_instancia* instancia;

	//Este seria el unico caso en el que te comunicas con la instancia que tiene la clave
	instancia = instancia_disponible_con_clave(clave);
	if (instancia != NULL) {
		log_info(logger, "La instancia %s tiene la clave %s", instancia->nombre,
				clave);
		char* valor = NULL;
		t_status_clave estado_instancia = instancia_solicitar_valor_de_clave(
				instancia, clave, &valor);
		switch (estado_instancia) {
		case INSTANCIA_CAIDA:
			log_warning(logger, "La instancia está caida");
			/* no break */
		case INSTANCIA_NO_TIENE_CLAVE:
			log_info(logger, "La instancia no posee un valor para dicha clave");
			agregar_status_a_paquete(paquete, NO_HAY_VALOR);
			break;
		case INSTANCIA_OK:
			log_info(logger, "El valor de la clave %s es %s", clave, valor);
			agregar_status_a_paquete(paquete, HAY_VALOR);
			paquete_agregar(paquete, valor, strlen(valor) + 1);
			break;
		default:
			;
			// esto nunca deberia pasar
		}
		agregar_status_a_paquete(paquete, estado_instancia);
		paquete_agregar(paquete, instancia->nombre,
				strlen(instancia->nombre) + 1);
		agregar_status_a_paquete(paquete, NO_HAY_SIMULACION);
		free(valor);
		return paquete;
	}

	//El caso en que la instancia que tiene la clave ya esté caida
	instancia = instancia_inactiva_con_clave(clave);
	if (instancia != NULL) {
		log_info(logger,
				"La instancia %s tiene la clave \"%s\", pero no está disponible",
				instancia->nombre, clave);
		agregar_status_a_paquete(paquete, NO_HAY_VALOR);
		agregar_status_a_paquete(paquete, INSTANCIA_CAIDA);
		paquete_agregar(paquete, instancia->nombre,
				strlen(instancia->nombre) + 1);
		agregar_status_a_paquete(paquete, NO_HAY_SIMULACION);
		return paquete;
	}

	//Caso en el que ninguna instancia tiene la clave y se realiza la simulacion
	log_info(logger, "Ninguna instancia tiene la clave %s", clave);
	agregar_status_a_paquete(paquete, NO_HAY_VALOR);
	agregar_status_a_paquete(paquete, INSTANCIA_NO_ASIGNADA);
	instancia = simular_algoritmo(clave);
	if (instancia != NULL) {
		log_info(logger,
				"Se elegiria a la instancia %s para almacenar la clave %s",
				instancia->nombre, clave);
		agregar_status_a_paquete(paquete, HAY_SIMULACION);
		paquete_agregar(paquete, instancia->nombre,
				strlen(instancia->nombre) + 1);
	} else {
		log_warning(logger,
				"No se pudo simular el algoritmo, tal vez no haya suficientes instancias disponibles");
		agregar_status_a_paquete(paquete, NO_HAY_SIMULACION);
	}

	return paquete;
}

void informar_status_clave(char* clave) {
	t_paquete* paquete = empaquetar_status_clave(clave);
	log_info(logger, "Enviando paquete de status clave al Planificador");

	if (paquete_enviar_con_codigo(paquete, RESPUESTA_STATUS_CLAVE,
			socket_planificador)) {
		paquete_destruir(paquete);
		exit_error_with_msg(
				"Error al enviar respuesta status clave al planificador");
	}

	log_info(logger, "Paquete enviado");
	paquete_destruir(paquete);

}
