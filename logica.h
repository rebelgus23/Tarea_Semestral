
#ifndef LOGICA_H
#define LOGICA_H

#include <time.h>

// Elementos del jardin.
#define PASTO 0
#define GIRASOL 1
#define LANZAGUISANTES 2
#define NUEZ 3
#define ZOMBIE 4
#define FLAG_ZOMBIE 5

// Para los costes de las plantas.
#define COSTO_GIRASOL 50
#define COSTO_LANZAGUISANTES 100
#define COSTO_NUEZ 150

// MISC
#define VIDA_NUEZ 5

// Soles generados.
#define SOLES_GENERADOS 25

typedef struct {

    int filas;
    int columnas;

    // Matrices del juego.
    int **mapa;
    int **vida;
    int **proyectiles;
    time_t **ultimo_evento;

    // Recursos de la partida.
    int soles;
    int vida_casa;
    int oleada_actual;

    // Frecuencias.
    int frecuencia_disparo;
    int frecuencia_soles;

    // Estado de la partida.
    int juego_activo;
    int victoria;

    // Control de generacion natural.
    time_t ultimo_solar;

} EstadoJuego;

// Inicializa las matrices y los valores.
int inicializar_juego(
    EstadoJuego *juego,
    int **mapa,
    int filas,
    int columnas
);

// Actualiza los sistemas del juego.
void actualizar_juego(
    EstadoJuego *juego,
    time_t tiempo_actual
);

// Coloca una planta en el jardin.
int colocar_planta(
    EstadoJuego *juego,
    int tipo,
    int fila,
    int columna
);

// Libera la memoria del juego.
void liberar_juego(EstadoJuego *juego);

#endif
