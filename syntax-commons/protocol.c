/*
 * protocol.c
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#include "protocol.h"

char* nombres_protocolo[] = {
	"EXITO",
	"ERROR",
	"SOLICITUD_CLAVE",
	"BLOQUEO_CLAVE",
	"DESBLOQUEO_CLAVE",
	"BLOQUEO_ESI",
	"CORRER_ESI",
	"ESI_TIENE_CLAVE",
	"FINALIZO_ESI",
	"OP_GET",
	"OP_SET",
	"OP_STORE",
	"MATAR_INSTANCIA",
	"INSTANCIA_CAIDA_EXCEPTION",
	"CLAVE_INACCESIBLE_EXCEPTION",
	"CLAVE_NO_BLOQUEADA_EXCEPTION",
	"CLAVE_NO_IDENTIFICADA_EXCEPTION",
	"LINEA_INVALIDA",
	"LINEA_SIZE",
	"INTERPRETAR",
	"ABORTA",
	"RELEVANTAR_INSTANCIA",
	"SOLICITUD_STATUS_CLAVE",
	"INSTANCIA_COMPACTAR",
	"ERROR_CONEXION"
};

char* to_string_protocolo(t_protocolo codigo){
	return nombres_protocolo[codigo];
}
