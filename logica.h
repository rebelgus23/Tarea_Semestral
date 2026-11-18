#ifndef LOGICA_H
#define LOGICA_H

#include <time.h>

#define VACIO 0
#define PLANTA 1
#define ZOMBIE_ENTIDAD 2

#define PASTO 0
#define GIRASOL 1
#define LANZAGUISANTES 2
#define NUEZ 3
#define ZOMBIE_NORMAL 4
#define FLAG_ZOMBIE 5

#define COSTO_GIRASOL 50
#define COSTO_LANZAGUISANTES 100
#define COSTO_NUEZ 150
#define VIDA_NUEZ 5
#define SOLES_GENERADOS 25

typedef struct {
    int filas;
    int columnas;

    int **tipo_entidad;
    int **clase_entidad;
    int **vida;
    int **proyectiles;

    time_t **ultimo_evento;
    time_t **ultimo_proyectil;
    time_t **ultimo_mov_zombie; 
    time_t ultimo_spawn; 

    int soles;
    int vida_casa;
    int oleada_actual;

    int frecuencia_disparo;
    int frecuencia_soles;

    int juego_activo;
    int victoria;

    time_t ultimo_solar;
} EstadoJuego;

int **crear_matriz(int filas, int columnas);
int inicializar_juego(EstadoJuego *juego, int **mapa, int filas, int columnas);
void actualizar_juego(EstadoJuego *juego, time_t tiempo_actual);
int colocar_planta(EstadoJuego *juego, int tipo, int fila, int columna);
void eliminar_planta(EstadoJuego *juego, int fila, int columna); // NUEVA
void guardar_partida(EstadoJuego *juego, const char *nombre);    // NUEVA
void liberar_juego(EstadoJuego *juego);

#endif