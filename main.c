#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "tarea1.h"
#include "logica.h"

#define ARCHIVO_GUARDADO "partida_guardada.txt" // Archivo compartido de guardado/carga

// Libera la matriz reservada para el mapa en consola
static void liberar_mapa_consola(int **mapa, int filas) {
    if (mapa == NULL) {
        return;
    }
    for (int i = 0; i < filas; i++) {
        free(mapa[i]);
    }
    free(mapa);
}

int main(void) {
    srand((unsigned int)time(NULL)); // Semilla para mapas aleatorios y oleadas
    int filas = 0;
    int columnas = 0;
    int elegir = 0;
    int **mapa = NULL;

    // Seleccion inicial: mapa predeterminado, mapa aleatorio o partida guardada
    printf("Elija que mapa quiere cargar. 1 (predeterminado), 2 (aleatorio) o 3 (cargar partida guardada):\n");
    while (scanf("%d", &elegir) != 1 || elegir < 1 || elegir > 3) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Entrada invalida. Ingrese 1, 2 o 3:\n");
    }

    leer_config();

    EstadoJuego juego;
    juego.juego_activo = 0;
    juego.victoria = 0;
    juego.tipo_entidad = NULL;
    juego.clase_entidad = NULL;
    juego.vida = NULL;
    juego.proyectiles = NULL;
    juego.ultimo_evento = NULL;
    juego.ultimo_proyectil = NULL;
    juego.ultimo_mov_zombie = NULL;

    if (elegir == 1) {
        mapa = cargar_mapa("mapa.txt", &filas, &columnas);
        if (mapa == NULL || !inicializar_juego(&juego, mapa, filas, columnas)) {
            printf("Error al cargar o inicializar mapa.txt\n");
            liberar_mapa_consola(mapa, filas);
            return 1;
        }
    } else if (elegir == 2) {
        do {
            printf("Ingrese las filas y columnas para el mapa al azar (minimo 3x3):\n");
            scanf("%d %d", &filas, &columnas);
        } while (filas < 3 || columnas < 3);
        generar_mapa_aleatorio(&filas, &columnas);
        mapa = cargar_mapa("mapa_aleatorio.txt", &filas, &columnas);
        if (mapa == NULL || !inicializar_juego(&juego, mapa, filas, columnas)) {
            printf("Error al cargar o inicializar mapa_aleatorio.txt\n");
            liberar_mapa_consola(mapa, filas);
            return 1;
        }
    } else {
        if (!cargar_partida(&juego, ARCHIVO_GUARDADO)) {
            printf("No se pudo cargar %s. Iniciando mapa predeterminado...\n", ARCHIVO_GUARDADO);
            mapa = cargar_mapa("mapa.txt", &filas, &columnas);
            if (mapa == NULL || !inicializar_juego(&juego, mapa, filas, columnas)) {
                liberar_mapa_consola(mapa, filas);
                return 1;
            }
        }
    }

    int eleccion = 0;
    int aux_fila = 0;
    int aux_col = 0;

    // Bucle principal por turnos en consola
    while (juego.juego_activo) {
        printf("\nSoles: %d | Vida Casa: %d | Oleada: %d/%d%s\n",
               juego.soles,
               juego.vida_casa,
               juego.oleada_actual,
               juego.max_oleadas,
               juego.en_descanso ? " (Descanso)" : "");

        // Imprime el tablero con proyectiles (*), plantas (S, L, N), zombies (z, Z) y vacio (.)
        for (int i = 0; i < juego.filas; i++) {
            printf("    ");
            for (int j = 0; j < juego.columnas; j++) {
                if (juego.proyectiles[i][j] == 1) {
                    printf("*");
                } else if (juego.clase_entidad[i][j] == PASTO) {
                    printf(".");
                } else if (juego.clase_entidad[i][j] == GIRASOL) {
                    printf("S");
                } else if (juego.clase_entidad[i][j] == LANZAGUISANTES) {
                    printf("L");
                } else if (juego.clase_entidad[i][j] == NUEZ) {
                    printf("N");
                } else if (juego.clase_entidad[i][j] == ZOMBIE_NORMAL) {
                    printf("z");
                } else if (juego.clase_entidad[i][j] == FLAG_ZOMBIE) {
                    printf("Z");
                }
            }
            printf("\n");
        }
        printf("\n");

        // Menu de acciones equivalentes a la interfaz grafica
        printf("Elige una opcion:\n");
        printf("(1) Plantar Girasol (%d soles)\n", COSTO_GIRASOL);
        printf("(2) Plantar Lanzaguisantes (%d soles)\n", COSTO_LANZAGUISANTES);
        printf("(3) Plantar Nuez (%d soles)\n", COSTO_NUEZ);
        printf("(4) Pasar turno\n");
        printf("(5) Eliminar planta (Pala)\n");
        printf("(6) Guardar partida\n");
        printf("(7) Cargar partida\n");
        scanf("%d", &eleccion);

        if (eleccion >= 1 && eleccion <= 3) {
            printf("Ingrese la fila y la columna (EN ESE ORDEN):\n");
            scanf("%d %d", &aux_fila, &aux_col);
            if (colocar_planta(&juego, eleccion, aux_fila, aux_col)) {
                printf("Planta colocada!\n");
            } else {
                printf("No se pudo colocar (casilla ocupada o faltan soles).\n");
            }
        } else if (eleccion == 5) {
            printf("Ingrese la fila y la columna de la planta a eliminar:\n");
            scanf("%d %d", &aux_fila, &aux_col);
            if (aux_fila >= 0 && aux_fila < juego.filas &&
                aux_col >= 0 && aux_col < juego.columnas &&
                juego.tipo_entidad[aux_fila][aux_col] == PLANTA) {
                eliminar_planta(&juego, aux_fila, aux_col);
                printf("Planta eliminada.\n");
            } else {
                printf("No hay planta en esa casilla.\n");
            }
        } else if (eleccion == 6) {
            if (guardar_partida(&juego, ARCHIVO_GUARDADO)) {
                printf("Partida guardada en %s.\n", ARCHIVO_GUARDADO);
            } else {
                printf("No se pudo guardar la partida.\n");
            }
        } else if (eleccion == 7) {
            if (cargar_partida(&juego, ARCHIVO_GUARDADO)) {
                printf("Partida cargada desde %s.\n", ARCHIVO_GUARDADO);
            } else {
                printf("No existe partida guardada.\n");
            }
        }

        // Actualiza soles, plantas, proyectiles, oleadas, zombies y fin de partida
        time_t ahora = time(NULL);
        actualizar_juego(&juego, ahora, (uint64_t)ahora * 1000ULL);

        printf("Presiona Enter para el siguiente turno\n");
        getchar();
        getchar();
    }

    // Resultado final de la partida
    if (juego.victoria) {
        printf("Victoria! Sobreviviste todas las oleadas.\n");
    } else {
        printf("Los Zombies Ganaron!!!\n");
    }

    // Libera memoria dinamica antes de salir
    liberar_juego(&juego);
    liberar_mapa_consola(mapa, filas);
    return 0;
}