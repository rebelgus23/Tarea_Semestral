#ifndef LOGICA_H
#define LOGICA_H

#include <time.h>
#include <stdint.h>

// Tipos generales de ocupacion en cada casilla
#define VACIO 0
#define PLANTA 1
#define ZOMBIE_ENTIDAD 2

// Clases especificas de entidades en el mapa y tablero
#define PASTO 0
#define GIRASOL 1
#define LANZAGUISANTES 2
#define NUEZ 3
#define ZOMBIE_NORMAL 4
#define FLAG_ZOMBIE 5

// Costos en soles y valores base
#define COSTO_GIRASOL 50
#define COSTO_LANZAGUISANTES 100
#define COSTO_NUEZ 150

#define VIDA_NUEZ 5
#define SOLES_GENERADOS 25

// Tiempos de dinamicas (en segundos y milisegundos)
#define INTERVALO_SPAWN_ZOMBIE 5
#define TIEMPO_DESCANSO_OLEADA 5
#define ZOMBIES_BASE_OLEADA 3
#define ZOMBIES_EXTRA_OLEADA 2
#define COOLDOWN_ZOMBIE 3
#define COOLDOWN_PROYECTIL_MS 250ULL // 250 ms por casilla para que el guisante avance fluido
#define DANO_PLANTA_NORMAL 1
#define DANO_NUEZ 2
#define DANO_CASA 1

// Estado completo de la partida activa
typedef struct {
    int filas;                    // Filas del jardin
    int columnas;                 // Columnas del jardin

    int **tipo_entidad;           // VACIO, PLANTA o ZOMBIE_ENTIDAD
    int **clase_entidad;          // PASTO, GIRASOL, LANZAGUISANTES, NUEZ, ZOMBIE_NORMAL, FLAG_ZOMBIE
    int **vida;                   // Vida actual de la entidad en (i, j)
    int **proyectiles;            // 1 si hay proyectil activo en (i, j), 0 si no

    time_t **ultimo_evento;       // Ultimo disparo o produccion de sol en (i, j)
    uint64_t **ultimo_proyectil;  // Marca en milisegundos del ultimo avance del proyectil en (i, j)
    time_t **ultimo_mov_zombie;   // Ultimo movimiento o mordida del zombie en (i, j)
    time_t ultimo_spawn;          // Ultimo intento de aparicion de zombie
    time_t fin_descanso;          // Marca de tiempo al iniciar descanso entre oleadas

    int soles;                    // Soles disponibles
    int vida_casa;                // Vida restante de la casa
    int oleada_actual;            // Oleada actual (1..max_oleadas)
    int max_oleadas;              // Total de oleadas a sobrevivir para ganar
    int zombies_generados_oleada; // Zombies ya generados en la oleada actual
    int zombies_objetivo_oleada;  // Total de zombies a generar en la oleada actual
    int en_descanso;              // 1 si esta en descanso entre oleadas, 0 si no

    int frecuencia_disparo;       // Segundos entre disparos de lanzaguisantes
    int frecuencia_soles;         // Segundos entre generaciones de soles

    int juego_activo;             // 1 mientras sigue la partida, 0 si termino
    int victoria;                 // 1 si sobrevivio todas las oleadas, 0 si perdio o sigue

    time_t ultimo_solar;          // Ultima generacion periodica de soles del jardin
} EstadoJuego;

// Manejo de matrices dinamicas
int **crear_matriz(int filas, int columnas);
time_t **crear_matriz_tiempo(int filas, int columnas);
uint64_t **crear_matriz_ms(int filas, int columnas);
void liberar_matriz(int **matriz, int filas);
void liberar_matriz_tiempo(time_t **matriz, int filas);
void liberar_matriz_ms(uint64_t **matriz, int filas);

// Ciclo principal de la partida
int inicializar_juego(EstadoJuego *juego, int **mapa, int filas, int columnas);
void actualizar_juego(EstadoJuego *juego, time_t tiempo_actual, uint64_t tiempo_ms);
int colocar_planta(EstadoJuego *juego, int tipo, int fila, int columna);
void eliminar_planta(EstadoJuego *juego, int fila, int columna);

// Subsistemas del juego
void generar_soles(EstadoJuego *juego, time_t tiempo_actual);
void actualizar_girasoles(EstadoJuego *juego, time_t tiempo_actual);
void actualizar_lanzaguisantes(EstadoJuego *juego, time_t tiempo_actual, uint64_t tiempo_ms);
void mover_proyectiles(EstadoJuego *juego, uint64_t tiempo_ms);
void generar_oleada(EstadoJuego *juego, time_t tiempo_actual);
void mover_zombies(EstadoJuego *juego, time_t tiempo_actual);
int quedan_zombies_en_mapa(const EstadoJuego *juego);
void comprobar_fin_juego(EstadoJuego *juego);

// Paquete 1: guardar y cargar partida desde archivo
int guardar_partida(const EstadoJuego *juego, const char *ruta_archivo);
int cargar_partida(EstadoJuego *juego, const char *ruta_archivo);

// Liberacion de memoria
void liberar_juego(EstadoJuego *juego);

#endif