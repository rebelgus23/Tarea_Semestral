
#ifndef LOGICA_H
#define LOGICA_H

#include <time.h>

// Tipos de entidades
#define VACIO 0
#define PLANTA 1
#define ZOMBIE_ENTIDAD 2

// Clases de entidades
#define PASTO 0
#define GIRASOL 1
#define LANZAGUISANTES 2
#define NUEZ 3
#define ZOMBIE_NORMAL 4
#define FLAG_ZOMBIE 5

// Costos y valores iniciales
#define COSTO_GIRASOL 50
#define COSTO_LANZAGUISANTES 100
#define COSTO_NUEZ 150
#define VIDA_NUEZ 5
#define SOLES_GENERADOS 25


// Estructura que almacena el estado actual de la partida
typedef struct {
    int filas;
    int columnas;

    int **tipo_entidad;
    int **clase_entidad;
    int **vida;
    int **proyectiles;

    time_t **ultimo_evento;
    time_t **ultimo_proyectil;

    int soles;
    int vida_casa;
    int oleada_actual;

    int frecuencia_disparo;
    int frecuencia_soles;

    int juego_activo;
    int victoria;

    time_t ultimo_solar;
} EstadoJuego;

// Crea una matriz dinamica inicializada en cero
int **crear_matriz(int filas, int columnas);

// Inicializa las matrices y los valores iniciales de la partida
int inicializar_juego(EstadoJuego *juego, int **mapa, int filas, int columnas);

// Ejecuta las actualizaciones correspondientes al estado del juego
void actualizar_juego(EstadoJuego *juego, time_t tiempo_actual);

// Comprueba si se puede colocar una planta y descuenta su costo
int colocar_planta(EstadoJuego *juego, int tipo, int fila, int columna);

// Genera soles de forma periodica
void generar_soles(EstadoJuego *juego, time_t tiempo_actual);

// Controla la generacion de soles de los girasoles
void actualizar_girasoles(EstadoJuego *juego, time_t tiempo_actual);

// Comprueba si los lanzaguisantes pueden disparar
void actualizar_lanzaguisantes(EstadoJuego *juego, time_t tiempo_actual);

// Avanza los proyectiles y comprueba si impactan contra un zombie
void mover_proyectiles(EstadoJuego *juego, time_t tiempo_actual);

// Libera la memoria utilizada por el estado del juego
void liberar_juego(EstadoJuego *juego);

#endif
