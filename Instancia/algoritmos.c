/*
 * algoritmos.c
 *
 *  Created on: 14 jul. 2018
 *      Author: utnso
 */

#include "algoritmos.h"

#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>

#include "almacenamiento.h"

/*
 * Saca la entrada de la tabla de entradas
 * Y te limpia el bitarray
 */
void liberar_entrada(void* unaEntrada) {
	tablaE* entrada = unaEntrada;
	remover_de_tabla(entrada->indice);
	log_debug(logger,
			"Se procede a liberar la entrada Nro: %d, que contiene la clave %s",
			entrada->indice, entrada->clave);
	int cant_entradas = entradas_que_ocupa(entrada);
	almac_liberar_entradas(entrada->indice, cant_entradas);
	liberarEntrada(entrada);
}
int cantidad_entradas_atomicas() {
	return list_count_satisfying(tabla, esAtomica);
}

void algoritmoCircular(claveEntrada* cv) {
	int entradas_necesarias = entradas_que_ocupa_por_tamanio(cv->tamanio);

	log_debug(logger,
			"Se necesitan %d entradas para reemplazar, hay %d entradas atómicas",
			entradas_necesarias, cantidad_entradas_atomicas());
	if (entradas_necesarias > cantidad_entradas_atomicas()) {
		log_error(logger,
				"No hay suficientes entradas atomicas para reemplazar");
		//aca hay que informar error al coordinador
		return;
	}

	t_list* entradas_a_reemplazar = list_create();

	int i;
	for (i = 0; i < entradas_necesarias; i++) {
		tablaE* entrada = obtener_siguiente_entrada_circular();
		if (entrada == NULL) {
			log_error(logger, "Esto no deberia pasar");
			return;
		}
		list_add(entradas_a_reemplazar, entrada);
		log_debug(logger,
				"Se reemplazara la entrada Nro: %d, que contenia la clave %s",
				entrada->indice, entrada->clave);
	}

	list_iterate(entradas_a_reemplazar, liberar_entrada);
	list_destroy(entradas_a_reemplazar);

	int posicion_a_insertar = almac_primera_posicion_libre_con_tamanio(
			entradas_necesarias);

	if (posicion_a_insertar < 0) {
		log_warning(logger, "Hay fragmentación externa");
		//Aca hay que solicitar compactacion al coordinador
		return;
	}
	agregarEnTabla(posicion_a_insertar, cv);
	setEnAlmacenamiento(posicion_a_insertar, cv->valor, cv->tamanio);
	log_info(logger, "Reemplazo ejecutado exitosamente!!!!");
}

int obtener_indice_de_entrada(t_list* lista_de_tablaE, int posicion) {
	int i;
	tablaE* elemento;
	for (i = 0; i < list_size(lista_de_tablaE); i++) {
		elemento = list_get(lista_de_tablaE, i);
		if (elemento->indice == posicion) {
			return i;
		}
	}
	log_error(logger, "No se pudo encontrar el elemento en la lista");
	return -1;
}

tablaE* primera_entrada_atomica() {
	return list_find(tabla, esAtomica);
}

/*
 * Devuelve la primera entrada atomica desde cierta posicion
 * Hace el "circulo"
 * Validar antes que haya entradas atomicas, porque sino devuelve null
 */
tablaE* primera_entrada_atomica_desde(int posicion_desde) {
	list_sort(tabla, ordenAscendente);
	tablaE* entrada;
	int i;

	for (i = 0; i < list_size(tabla); i++) {
		entrada = list_get(tabla, i);
		if (esAtomica(entrada) && entrada->indice >= posicion_desde) {
			return entrada;
		}
	}

	return primera_entrada_atomica();
}

/*
 * Devuelve la siguiente entrada que seria reemplazada por el algoritmo circular
 * Avanza el puntero a la ultima entrada que devolvio
 * IMPORTANTE: validar antes que haya entradas atomicas disponibles
 */
tablaE* obtener_siguiente_entrada_circular() {
	tablaE* entrada = primera_entrada_atomica_desde(posicion);
	posicion = entrada->indice + 1;
	if(posicion >= obtenerEntradasTotales())
		posicion = 0;
	return entrada;
}

void algoritmoBSU(claveEntrada*cv){
	log_trace(logger, "Se procede a reemplazar mediante algoritmo BSU");
	int entradas_necesarias = entradas_que_ocupa_por_tamanio(cv->tamanio);
	int entradas_ocupa=0;
	log_debug(logger,"Se necesitan %d entradas para reemplazar, hay %d entradas atómicas",entradas_necesarias, cantidad_entradas_atomicas());
	if (entradas_necesarias > cantidad_entradas_atomicas()) {
			log_error(logger,"No hay suficientes entradas atomicas para reemplazar");
			//aca hay que informar error al coordinador
			return;
	}
	t_list* entradas_a_reemplazar = list_create();

	for(int i=0; i<list_size(tabla);i++){
		tablaE* entrada=list_get(tabla,i);
		 entradas_ocupa += entradas_que_ocupa_por_tamanio(entrada->tamanio);
		log_trace(logger,"se obtuvo la clave (%s) de la entrada numero (%d) en la iteracion (%s)", entrada->clave,entrada->indice,i);
		if(esAtomica(entrada) && entradas_ocupa<=entradas_necesarias){
			log_trace(logger,"la entrdada (%s) es atomica", entrada->clave);
			list_add(entradas_a_reemplazar,entrada);

		}
	}
	list_sort(entradas_a_reemplazar,ordenDescendentePorTamanio);
	list_iterate((list_take(entradas_a_reemplazar,entradas_necesarias)), liberar_entrada);
	list_destroy(entradas_a_reemplazar);

	int posicion_a_insertar = almac_primera_posicion_libre_con_tamanio(entradas_necesarias);
	if (posicion_a_insertar < 0) {
		log_warning(logger, "Hay fragmentación externa");
		//Aca hay que solicitar compactacion al coordinador
		return;
	}
	agregarEnTabla(posicion_a_insertar, cv);
	setEnAlmacenamiento(posicion_a_insertar, cv->valor, cv->tamanio);
	log_info(logger, "Reemplazo ejecutado exitosamente!!!!");


}


void algoritmoLRU(claveEntrada* cv) {
	log_trace(logger, "Se procede a reemplazar mediante algoritmo LRU");
	t_list* posiblesAReemplazar = list_duplicate(tabla);
	list_sort(posiblesAReemplazar, ordenAscendentePorOperacion);
	log_trace(logger,
			"Se procede a chequear que la cantidad de entradas a reemplazar alcance para el tamaño del cv");
	if (list_size(posiblesAReemplazar) >= cv->tamanio) {
		log_trace(logger,
				"Alcanzaban: La cantidad de posiblesAReemplazar es %d, el tamaño del cv es: %d",
				list_size(posiblesAReemplazar), cv->tamanio);
		t_list* vanASerReemplazadas = list_take_and_remove(posiblesAReemplazar,
				cv->tamanio / obtenerTamanioEntrada());
		if (listaNoContigua(vanASerReemplazadas)) {
			// todo: compactar
		}
		log_trace(logger,
				"Se ṕrocede a obtener el listado final de claves a ser reemplazadas");
		t_list* clavesAReemplazar = list_map(vanASerReemplazadas, obtenerClave);
		log_trace(logger,
				"se procede a eliminar las claves a ser reemplazadas");
		for (int i = 0; i < list_size(clavesAReemplazar); i++) {
			char* clave = list_get(clavesAReemplazar, i);
			log_trace(logger, "se obtuvo la clave (%s)", clave);
			tablaE* unaEntrada = buscarEntrada(clave);
			liberarEntrada(unaEntrada);
			free(clave);
			log_trace(logger, "se elimino la clave de la tabla de entradas");
		}
		hacer_set(cv->clave, cv->valor);
		list_destroy(posiblesAReemplazar);
		list_destroy(vanASerReemplazadas);
		list_destroy(clavesAReemplazar);
		return;
	}
	list_destroy(posiblesAReemplazar);
	log_error(logger, "No se pudo encontrar clave a reemplazar");
}



bool esAtomica(void* unaEntrada) {
	tablaE* entrada = unaEntrada;
	return entrada->tamanio <= obtenerTamanioEntrada();
}

bool listaNoContigua(t_list* unaLista) {
	for (int i = 0; i < (list_size(unaLista) - 1); i++) {
		tablaE* unaEntrada = list_get(unaLista, i);
		tablaE* sigEntrada = list_get(unaLista, i + 1);
		if ((unaEntrada->indice + entradas_que_ocupa(unaEntrada))
				!= sigEntrada->indice - 1) {
			return true;
		}
	}
	return false;
}

