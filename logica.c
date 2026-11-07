
#include <stdio.h>
#include <stdlib.h>
#include "logica.h"
#include "tarea1.h"

// reserva memoria para una matriz de enteros.
int **crear_matriz(int filas, int columnas) {

    int **matriz = malloc(filas * sizeof(int *));

    if (matriz == NULL) {
        return NULL;
    }

    for (int i = 0; i < filas; i++) {

        matriz[i] = calloc(columnas, sizeof(int));

        if (matriz[i] == NULL) {

            for (int j = 0; j < i; j++) {
                free(matriz[j]);
            }

            free(matriz);
            return NULL;
        }
    }

    return matriz;
}

// Reserva memoria para Matriz de tiempo
time_t **crear_matriz_tiempo(int filas, int columnas) {

    time_t **matriz = malloc(filas * sizeof(time_t *));

    if (matriz == NULL) {
        return NULL;
    }

    for (int i = 0; i < filas; i++) {

        matriz[i] = calloc(columnas, sizeof(time_t));

        if (matriz[i] == NULL) {

            for (int j = 0; j < i; j++) {
                free(matriz[j]);
            }

            free(matriz);
            return NULL;
        }
    }

    return matriz;
}

// Libera una Matriz de enteros
void liberar_matriz(int **matriz, int filas) {

    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < filas; i++) {
        free(matriz[i]);
    }

    free(matriz);
}

// Libera una Matriz de tiempo
void liberar_matriz_tiempo(time_t **matriz, int filas) {

    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < filas; i++) {
        free(matriz[i]);
    }

    free(matriz);
}

// Inicializa el estado del juego.
int inicializar_juego(
    EstadoJuego *juego,
    int **mapa,
    int filas,
    int columnas
) {

    juego->filas = filas;
    juego->columnas = columnas;

    // La matriz recibida pasa a ser propiedad del juego
    juego->mapa = mapa;

    // Se reservan las matrices adicionales
    juego->vida = crear_matriz(filas, columnas);

    juego->proyectiles = crear_matriz(filas, columnas);

    juego->ultimo_evento = crear_matriz_tiempo(filas, columnas);

    if (juego->vida == NULL ||
        juego->proyectiles == NULL ||
        juego->ultimo_evento == NULL) {

        liberar_matriz(juego->vida, filas);
        liberar_matriz(juego->proyectiles, filas);
        liberar_matriz_tiempo(juego->ultimo_evento, filas);

        juego->vida = NULL;
        juego->proyectiles = NULL;
        juego->ultimo_evento = NULL;
        juego->mapa = NULL;

        return 0;
    }

    // Inicializa la vida de los objetos
    for (int i = 0; i < filas; i++) {

        for (int j = 0; j < columnas; j++) {

            switch (mapa[i][j]) {

                case GIRASOL:
                case LANZAGUISANTES:
                    juego->vida[i][j] = 1;
                    break;

                case NUEZ:
                    juego->vida[i][j] = VIDA_NUEZ;
                    break;

                case ZOMBIE:
                case FLAG_ZOMBIE:
                    juego->vida[i][j] = vida_Zombies;
                    break;

                default:
                    juego->vida[i][j] = 0;
            }
        }
    }

    // Se inicializan los recursos.
    juego->soles = 50;
    juego->vida_casa = vida_Casa;
    juego->oleada_actual = 0;

    juego->frecuencia_disparo = frecuencia_Disparo_Plantas;
    juego->frecuencia_soles = frecuencia_Soles;

    juego->juego_activo = 1;
    juego->victoria = 0;

    // Se registra el tiempo inicial.
    time_t ahora = time(NULL);

    juego->ultimo_solar = ahora;

    for (int i = 0; i < filas; i++) {

        for (int j = 0; j < columnas; j++) {
            juego->ultimo_evento[i][j] = ahora;
        }
    }

    return 1;
}

// coloca una planta en una posicion vacia.
int colocar_planta(
    EstadoJuego *juego,
    int tipo,
    int fila,
    int columna
) {

    if (fila < 0 || fila >= juego->filas ||
        columna < 0 || columna >= juego->columnas) {

        return 0;
    }

    if (juego->mapa[fila][columna] != PASTO) {
        return 0;
    }

    int costo;

    if (tipo == GIRASOL) {
        costo = COSTO_GIRASOL;
    }
    else if (tipo == LANZAGUISANTES) {
        costo = COSTO_LANZAGUISANTES;
    }
    else if (tipo == NUEZ) {
        costo = COSTO_NUEZ;
    }
    else {
        return 0;
    }

    if (juego->soles < costo) {
        return 0;
    }

    // Se descuentan los soles y se coloca la planta.
    juego->soles -= costo;

    juego->mapa[fila][columna] = tipo;

    if (tipo == NUEZ) {
        juego->vida[fila][columna] = VIDA_NUEZ;
    }
    else {
        juego->vida[fila][columna] = 1;
    }

    juego->ultimo_evento[fila][columna] = time(NULL);

    return 1;
}
