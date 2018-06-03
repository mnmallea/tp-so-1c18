#include "deserializador.h"

void configure_logger_deser() {
  logger = log_create("serializador.log", "serializador",true, LOG_LEVEL_INFO);
}


int recibirPaquete(int my_socket, void** id, size_t tamanio) {
	int res_recv = recv(my_socket, id, tamanio, MSG_WAITALL);

	if(res_recv < 0) {
		salir_con_error(my_socket,"Error al recibir la información");
	}

	log_info(logger, "Mensaje recibido");
	return res_recv;
}

int recibirPaqueteVariable(int my_socket, void** id) {
	int res_recv;
	size_t tamanio;


	res_recv = recibirPaquete(my_socket, &tamanio, sizeof(tamanio));

	if(res_recv < 0) {
		salir_con_error(my_socket,"Error al recibir el tamanio de la información");
	}

	if((id=malloc(tamanio)) == NULL) {
		salir_con_error(my_socket,"Error al realizar el malloc");
	}
	res_recv = recibirPaquete(my_socket, *id, tamanio);
	return res_recv;
}