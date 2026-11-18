#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "logica.h"
#include "tarea1.h"

int **crear_matriz(int filas, int columnas) {
    int **matriz = malloc(filas * sizeof(int *));
    if (matriz == NULL) return NULL;
    for (int i = 0; i < filas; i++) {
        matriz[i] = calloc(columnas, sizeof(int));
        if (matriz[i] == NULL) {
            for (int j = 0; j < i; j++) free(matriz[j]);
            free(matriz);
            return NULL;
        }
    }
    return matriz;
}

time_t **crear_matriz_tiempo(int filas, int columnas) {
    time_t **matriz = malloc(filas * sizeof(time_t *));
    if (matriz == NULL) return NULL;
    for (int i = 0; i < filas; i++) {
        matriz[i] = calloc(columnas, sizeof(time_t));
        if (matriz[i] == NULL) {
            for (int j = 0; j < i; j++) free(matriz[j]);
            free(matriz);
            return NULL;
        }
    }
    return matriz;
}

void liberar_matriz(int **matriz, int filas) {
    if (matriz == NULL) return;
    for (int i = 0; i < filas; i++) free(matriz[i]);
    free(matriz);
}

void liberar_matriz_tiempo(time_t **matriz, int filas) {
    if (matriz == NULL) return;
    for (int i = 0; i < filas; i++) free(matriz[i]);
    free(matriz);
}

int inicializar_juego(EstadoJuego *juego, int **mapa, int filas, int columnas) {
    if (juego == NULL || mapa == NULL || filas <= 0 || columnas <= 0) return 0;

    juego->filas = filas;
    juego->columnas = columnas;

    juego->tipo_entidad = crear_matriz(filas, columnas);
    juego->vida = crear_matriz(filas, columnas);
    juego->proyectiles = crear_matriz(filas, columnas);
    juego->ultimo_evento = crear_matriz_tiempo(filas, columnas);
    juego->ultimo_proyectil = crear_matriz_tiempo(filas, columnas);
    juego->ultimo_mov_zombie = crear_matriz_tiempo(filas, columnas);

    if (juego->tipo_entidad == NULL || juego->vida == NULL || juego->proyectiles == NULL || 
        juego->ultimo_evento == NULL || juego->ultimo_proyectil == NULL || juego->ultimo_mov_zombie == NULL) {
        liberar_matriz(juego->tipo_entidad, filas); liberar_matriz(juego->vida, filas);
        liberar_matriz(juego->proyectiles, filas); liberar_matriz_tiempo(juego->ultimo_evento, filas);
        liberar_matriz_tiempo(juego->ultimo_proyectil, filas); liberar_matriz_tiempo(juego->ultimo_mov_zombie, filas);
        return 0;
    }

    juego->clase_entidad = mapa;

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int tipo = mapa[i][j];
            if (tipo == PASTO) { juego->tipo_entidad[i][j] = VACIO; juego->vida[i][j] = 0; }
            else if (tipo == GIRASOL || tipo == LANZAGUISANTES) { juego->tipo_entidad[i][j] = PLANTA; juego->vida[i][j] = 1; }
            else if (tipo == NUEZ) { juego->tipo_entidad[i][j] = PLANTA; juego->vida[i][j] = VIDA_NUEZ; }
            else if (tipo == ZOMBIE_NORMAL || tipo == FLAG_ZOMBIE) { juego->tipo_entidad[i][j] = ZOMBIE_ENTIDAD; juego->vida[i][j] = vida_Zombies; }
            else { return 0; }
        }
    }

    juego->soles = 50;
    juego->vida_casa = vida_Casa;
    juego->oleada_actual = 0;
    juego->frecuencia_disparo = frecuencia_Disparo_Plantas;
    juego->frecuencia_soles = frecuencia_Soles;
    juego->juego_activo = 1;
    juego->victoria = 0;
    juego->ultimo_solar = time(NULL);
    juego->ultimo_spawn = time(NULL) - 4; // Hace que el primer zombie salga en 1 seg

    return 1;
}

int colocar_planta(EstadoJuego *juego, int tipo, int fila, int columna) {
    if (juego == NULL || fila < 0 || fila >= juego->filas || columna < 0 || columna >= juego->columnas) return 0;
    if (juego->tipo_entidad[fila][columna] != VACIO) return 0;

    int costo, vida;
    if (tipo == GIRASOL) { costo = COSTO_GIRASOL; vida = 1; }
    else if (tipo == LANZAGUISANTES) { costo = COSTO_LANZAGUISANTES; vida = 1; }
    else if (tipo == NUEZ) { costo = COSTO_NUEZ; vida = VIDA_NUEZ; }
    else return 0;

    if (juego->soles < costo) return 0;

    juego->soles -= costo;
    juego->tipo_entidad[fila][columna] = PLANTA;
    juego->clase_entidad[fila][columna] = tipo;
    juego->vida[fila][columna] = vida;
    juego->ultimo_evento[fila][columna] = time(NULL);
    return 1;
}

// NUEVA FUNCION: Elimina una planta de la casilla
void eliminar_planta(EstadoJuego *juego, int fila, int columna) {
    if (fila >= 0 && fila < juego->filas && columna >= 0 && columna < juego->columnas) {
        if (juego->tipo_entidad[fila][columna] == PLANTA) {
            juego->tipo_entidad[fila][columna] = VACIO;
            juego->clase_entidad[fila][columna] = PASTO;
            juego->vida[fila][columna] = 0;
        }
    }
}

// NUEVA FUNCION: Guarda la matriz actual en un archivo
void guardar_partida(EstadoJuego *juego, const char *nombre) {
    FILE *archivo = fopen(nombre, "w");
    if (archivo == NULL) return;

    fprintf(archivo, "%d %d\n", juego->filas, juego->columnas);
    fprintf(archivo, "%d %d\n", juego->soles, juego->vida_casa); // Guardamos stats

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            fprintf(archivo, "%d ", juego->clase_entidad[i][j]);
        }
        fprintf(archivo, "\n");
    }
    fclose(archivo);
    printf("\n¡Partida guardada exitosamente en %s!\n", nombre);
}

void generar_soles(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego->frecuencia_soles <= 0) return;
    if (tiempo_actual - juego->ultimo_solar >= juego->frecuencia_soles) {
        juego->soles += SOLES_GENERADOS;
        juego->ultimo_solar = tiempo_actual;
    }
}

void actualizar_girasoles(EstadoJuego *juego, time_t tiempo_actual) {
    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            if (juego->clase_entidad[i][j] == GIRASOL && juego->frecuencia_soles > 0) {
                if (tiempo_actual - juego->ultimo_evento[i][j] >= juego->frecuencia_soles) {
                    juego->soles += SOLES_GENERADOS;
                    juego->ultimo_evento[i][j] = tiempo_actual;
                }
            }
        }
    }
}

void actualizar_lanzaguisantes(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego->frecuencia_disparo <= 0) return;
    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            if (juego->clase_entidad[i][j] != LANZAGUISANTES) continue;
            if (tiempo_actual - juego->ultimo_evento[i][j] < juego->frecuencia_disparo) continue;

            int hay_zombie = 0;
            for (int k = j + 1; k < juego->columnas; k++) {
                if (juego->tipo_entidad[i][k] == ZOMBIE_ENTIDAD) { hay_zombie = 1; break; }
            }
            if (!hay_zombie) continue;

            int siguiente = j + 1;
            if (siguiente >= juego->columnas) continue;

            if (juego->proyectiles[i][siguiente] == 0) {
                juego->proyectiles[i][siguiente] = 1;
                juego->ultimo_proyectil[i][siguiente] = tiempo_actual;
                juego->ultimo_evento[i][j] = tiempo_actual;
            }
        }
    }
}

void mover_proyectiles(EstadoJuego *juego, time_t tiempo_actual) {
    for (int i = 0; i < juego->filas; i++) {
        for (int j = juego->columnas - 1; j >= 0; j--) {
            if (juego->proyectiles[i][j] == 0) continue;
            if (tiempo_actual - juego->ultimo_proyectil[i][j] < 1) continue;

            juego->proyectiles[i][j] = 0;
            int siguiente = j + 1;
            if (siguiente >= juego->columnas) continue;

            if (juego->tipo_entidad[i][siguiente] == ZOMBIE_ENTIDAD) {
                juego->vida[i][siguiente]--;
                if (juego->vida[i][siguiente] <= 0) {
                    juego->tipo_entidad[i][siguiente] = VACIO;
                    juego->clase_entidad[i][siguiente] = PASTO;
                }
            } else {
                if (juego->proyectiles[i][siguiente] == 0) {
                    juego->proyectiles[i][siguiente] = 1;
                    juego->ultimo_proyectil[i][siguiente] = tiempo_actual;
                }
            }
        }
    }
}

void generar_oleada(EstadoJuego *juego, time_t tiempo_actual) {
    if (tiempo_actual - juego->ultimo_spawn >= 5) {
        int fila_random = rand() % juego->filas;
        int columna_final = juego->columnas - 1;

        if (juego->tipo_entidad[fila_random][columna_final] == VACIO) {
            juego->tipo_entidad[fila_random][columna_final] = ZOMBIE_ENTIDAD;
            juego->clase_entidad[fila_random][columna_final] = ZOMBIE_NORMAL;
            juego->vida[fila_random][columna_final] = vida_Zombies;
            juego->ultimo_mov_zombie[fila_random][columna_final] = tiempo_actual;
        }
        juego->ultimo_spawn = tiempo_actual;
    }
}

void mover_zombies(EstadoJuego *juego, time_t tiempo_actual) {
    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) { 
            if (juego->tipo_entidad[i][j] == ZOMBIE_ENTIDAD) {
                if (tiempo_actual - juego->ultimo_mov_zombie[i][j] >= 3) {
                    if (j == 0) {
                        juego->vida_casa--;
                        juego->tipo_entidad[i][j] = VACIO;
                        juego->clase_entidad[i][j] = PASTO;
                    } else {
                        if (juego->tipo_entidad[i][j-1] == VACIO) {
                            juego->tipo_entidad[i][j-1] = ZOMBIE_ENTIDAD;
                            juego->clase_entidad[i][j-1] = juego->clase_entidad[i][j];
                            juego->vida[i][j-1] = juego->vida[i][j];
                            juego->ultimo_mov_zombie[i][j-1] = tiempo_actual; 
                            
                            juego->tipo_entidad[i][j] = VACIO;
                            juego->clase_entidad[i][j] = PASTO;
                        } else if (juego->tipo_entidad[i][j-1] == PLANTA) {
                            int danio = 1;
                            if (juego->clase_entidad[i][j-1] == NUEZ) { danio = 2; }
                            
                            juego->vida[i][j-1] -= danio;
                            juego->ultimo_mov_zombie[i][j] = tiempo_actual;
                            
                            if (juego->vida[i][j-1] <= 0) {
                                juego->tipo_entidad[i][j-1] = VACIO;
                                juego->clase_entidad[i][j-1] = PASTO;
                            }
                        }
                    }
                }
            }
        }
    }
}

void actualizar_juego(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego == NULL || juego->juego_activo == 0) return;

    generar_soles(juego, tiempo_actual);
    actualizar_girasoles(juego, tiempo_actual);
    actualizar_lanzaguisantes(juego, tiempo_actual);
    mover_proyectiles(juego, tiempo_actual);
    generar_oleada(juego, tiempo_actual); 
    mover_zombies(juego, tiempo_actual); 
}

void liberar_juego(EstadoJuego *juego) {
    if (juego == NULL) return;
    liberar_matriz(juego->clase_entidad, juego->filas);
    liberar_matriz(juego->tipo_entidad, juego->filas);
    liberar_matriz(juego->vida, juego->filas);
    liberar_matriz(juego->proyectiles, juego->filas);
    liberar_matriz_tiempo(juego->ultimo_evento, juego->filas);
    liberar_matriz_tiempo(juego->ultimo_proyectil, juego->filas);
    liberar_matriz_tiempo(juego->ultimo_mov_zombie, juego->filas);
}