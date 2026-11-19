#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "logica.h"
#include "tarea1.h"

// Deja todos los punteros y campos de EstadoJuego en valores limpios
static void poner_a_cero(EstadoJuego *juego) {
    juego->filas = 0;
    juego->columnas = 0;
    juego->tipo_entidad = NULL;
    juego->clase_entidad = NULL;
    juego->vida = NULL;
    juego->proyectiles = NULL;
    juego->ultimo_evento = NULL;
    juego->ultimo_proyectil = NULL;
    juego->ultimo_mov_zombie = NULL;
    juego->ultimo_spawn = 0;
    juego->fin_descanso = 0;
    juego->soles = 0;
    juego->vida_casa = 0;
    juego->oleada_actual = 0;
    juego->max_oleadas = 0;
    juego->zombies_generados_oleada = 0;
    juego->zombies_objetivo_oleada = 0;
    juego->en_descanso = 0;
    juego->frecuencia_disparo = 0;
    juego->frecuencia_soles = 0;
    juego->juego_activo = 0;
    juego->victoria = 0;
    juego->ultimo_solar = 0;
}

// Calcula cuantos zombies corresponden a una oleada dada
static int zombies_para_oleada(int num_oleada) {
    if (num_oleada <= 1) {
        return ZOMBIES_BASE_OLEADA;
    }
    return ZOMBIES_BASE_OLEADA + (num_oleada - 1) * ZOMBIES_EXTRA_OLEADA;
}

// Reconstruye tipo_entidad y marcas de tiempo tras cargar una partida o mapa
static void sincronizar_entidades_desde_clase(EstadoJuego *juego, int vida_z, time_t ahora) {
    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            int clase = juego->clase_entidad[i][j];
            juego->ultimo_evento[i][j] = ahora;
            juego->ultimo_proyectil[i][j] = 0;
            juego->ultimo_mov_zombie[i][j] = ahora;

            if (clase == PASTO) {
                juego->tipo_entidad[i][j] = VACIO;
                juego->vida[i][j] = 0;
            } else if (clase == GIRASOL || clase == LANZAGUISANTES) {
                juego->tipo_entidad[i][j] = PLANTA;
                if (juego->vida[i][j] <= 0) {
                    juego->vida[i][j] = 1;
                }
            } else if (clase == NUEZ) {
                juego->tipo_entidad[i][j] = PLANTA;
                if (juego->vida[i][j] <= 0) {
                    juego->vida[i][j] = VIDA_NUEZ;
                }
            } else if (clase == ZOMBIE_NORMAL || clase == FLAG_ZOMBIE) {
                juego->tipo_entidad[i][j] = ZOMBIE_ENTIDAD;
                if (juego->vida[i][j] <= 0) {
                    juego->vida[i][j] = vida_z;
                }
            } else {
                juego->clase_entidad[i][j] = PASTO;
                juego->tipo_entidad[i][j] = VACIO;
                juego->vida[i][j] = 0;
            }
        }
    }
}

// Reserva memoria dinamica para una matriz de enteros inicializada en 0
int **crear_matriz(int filas, int columnas) {
    if (filas <= 0 || columnas <= 0) {
        return NULL;
    }

    int **matriz = malloc((size_t)filas * sizeof(int *));
    if (matriz == NULL) {
        return NULL;
    }

    for (int i = 0; i < filas; i++) {
        matriz[i] = calloc((size_t)columnas, sizeof(int));
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

// Reserva memoria dinamica para una matriz de marcas de tiempo (time_t)
time_t **crear_matriz_tiempo(int filas, int columnas) {
    if (filas <= 0 || columnas <= 0) {
        return NULL;
    }

    time_t **matriz = malloc((size_t)filas * sizeof(time_t *));
    if (matriz == NULL) {
        return NULL;
    }

    for (int i = 0; i < filas; i++) {
        matriz[i] = calloc((size_t)columnas, sizeof(time_t));
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

// Reserva memoria dinamica para una matriz de marcas en milisegundos (uint64_t)
uint64_t **crear_matriz_ms(int filas, int columnas) {
    if (filas <= 0 || columnas <= 0) {
        return NULL;
    }

    uint64_t **matriz = malloc((size_t)filas * sizeof(uint64_t *));
    if (matriz == NULL) {
        return NULL;
    }

    for (int i = 0; i < filas; i++) {
        matriz[i] = calloc((size_t)columnas, sizeof(uint64_t));
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

// Libera una matriz dinamica de enteros
void liberar_matriz(int **matriz, int filas) {
    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < filas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// Libera una matriz dinamica de tiempos (time_t)
void liberar_matriz_tiempo(time_t **matriz, int filas) {
    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < filas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// Libera una matriz dinamica de milisegundos (uint64_t)
void liberar_matriz_ms(uint64_t **matriz, int filas) {
    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < filas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// Inicializa una partida nueva a partir del mapa cargado
int inicializar_juego(EstadoJuego *juego, int **mapa, int filas, int columnas) {
    if (juego == NULL || mapa == NULL || filas <= 0 || columnas <= 0) {
        return 0;
    }

    int vida_z = (vida_Zombies > 0) ? vida_Zombies : 3;
    int vida_c = (vida_Casa > 0) ? vida_Casa : 5;
    int freq_disp = (frecuencia_Disparo_Plantas > 0) ? frecuencia_Disparo_Plantas : 2;
    int freq_sol = (frecuencia_Soles > 0) ? frecuencia_Soles : 5;
    int max_ol = (oleadas > 0) ? oleadas : 3;

    for (int i = 0; i < filas; i++) {
        if (mapa[i] == NULL) {
            return 0;
        }
        for (int j = 0; j < columnas; j++) {
            int tipo = mapa[i][j];
            if (tipo < PASTO || tipo > FLAG_ZOMBIE) {
                return 0;
            }
        }
    }

    poner_a_cero(juego);
    juego->filas = filas;
    juego->columnas = columnas;

    juego->tipo_entidad = crear_matriz(filas, columnas);
    juego->clase_entidad = crear_matriz(filas, columnas);
    juego->vida = crear_matriz(filas, columnas);
    juego->proyectiles = crear_matriz(filas, columnas);
    juego->ultimo_evento = crear_matriz_tiempo(filas, columnas);
    juego->ultimo_proyectil = crear_matriz_ms(filas, columnas);
    juego->ultimo_mov_zombie = crear_matriz_tiempo(filas, columnas);

    if (juego->tipo_entidad == NULL ||
        juego->clase_entidad == NULL ||
        juego->vida == NULL ||
        juego->proyectiles == NULL ||
        juego->ultimo_evento == NULL ||
        juego->ultimo_proyectil == NULL ||
        juego->ultimo_mov_zombie == NULL) {
        liberar_juego(juego);
        return 0;
    }

    time_t ahora = time(NULL);

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int tipo = mapa[i][j];
            juego->clase_entidad[i][j] = tipo;
            juego->vida[i][j] = 0;
            if (tipo == GIRASOL || tipo == LANZAGUISANTES) {
                juego->vida[i][j] = 1;
            } else if (tipo == NUEZ) {
                juego->vida[i][j] = VIDA_NUEZ;
            } else if (tipo == ZOMBIE_NORMAL || tipo == FLAG_ZOMBIE) {
                juego->vida[i][j] = vida_z;
            }
        }
    }

    sincronizar_entidades_desde_clase(juego, vida_z, ahora);

    juego->soles = 50;
    juego->vida_casa = vida_c;
    juego->oleada_actual = 1;
    juego->max_oleadas = max_ol;
    juego->zombies_generados_oleada = 0;
    juego->zombies_objetivo_oleada = zombies_para_oleada(1);
    juego->en_descanso = 0;
    juego->frecuencia_disparo = freq_disp;
    juego->frecuencia_soles = freq_sol;
    juego->juego_activo = 1;
    juego->victoria = 0;
    juego->ultimo_solar = ahora;
    juego->ultimo_spawn = ahora;
    juego->fin_descanso = 0;

    return 1;
}

// Coloca una planta en (fila, columna) si la casilla esta vacia y alcanza el costo
int colocar_planta(EstadoJuego *juego, int tipo, int fila, int columna) {
    if (juego == NULL || juego->juego_activo == 0) {
        return 0;
    }
    if (fila < 0 || fila >= juego->filas || columna < 0 || columna >= juego->columnas) {
        return 0;
    }
    if (juego->tipo_entidad[fila][columna] != VACIO) {
        return 0;
    }

    int costo = 0;
    int vida = 0;

    if (tipo == GIRASOL) {
        costo = COSTO_GIRASOL;
        vida = 1;
    } else if (tipo == LANZAGUISANTES) {
        costo = COSTO_LANZAGUISANTES;
        vida = 1;
    } else if (tipo == NUEZ) {
        costo = COSTO_NUEZ;
        vida = VIDA_NUEZ;
    } else {
        return 0;
    }

    if (juego->soles < costo) {
        return 0;
    }

    juego->soles -= costo;
    juego->tipo_entidad[fila][columna] = PLANTA;
    juego->clase_entidad[fila][columna] = tipo;
    juego->vida[fila][columna] = vida;
    juego->ultimo_evento[fila][columna] = time(NULL);

    return 1;
}

// Suma soles periodicos del jardin
void generar_soles(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego == NULL || juego->frecuencia_soles <= 0) {
        return;
    }
    if (tiempo_actual - juego->ultimo_solar >= juego->frecuencia_soles) {
        juego->soles += SOLES_GENERADOS;
        juego->ultimo_solar = tiempo_actual;
    }
}

// Cada girasol genera soles cada frecuencia_soles segundos
void actualizar_girasoles(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego == NULL || juego->frecuencia_soles <= 0) {
        return;
    }

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            if (juego->clase_entidad[i][j] == GIRASOL) {
                if (tiempo_actual - juego->ultimo_evento[i][j] >= juego->frecuencia_soles) {
                    juego->soles += SOLES_GENERADOS;
                    juego->ultimo_evento[i][j] = tiempo_actual;
                }
            }
        }
    }
}

// Los lanzaguisantes disparan si hay algun zombie a su derecha en la misma fila.
// Los proyectiles atraviesan casillas vacias o con otras plantas sin bloquearse.
void actualizar_lanzaguisantes(EstadoJuego *juego, time_t tiempo_actual, uint64_t tiempo_ms) {
    if (juego == NULL || juego->frecuencia_disparo <= 0) {
        return;
    }

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            if (juego->clase_entidad[i][j] != LANZAGUISANTES) {
                continue;
            }
            if (tiempo_actual - juego->ultimo_evento[i][j] < juego->frecuencia_disparo) {
                continue;
            }

            int hay_zombie = 0;
            for (int k = j + 1; k < juego->columnas; k++) {
                if (juego->tipo_entidad[i][k] == ZOMBIE_ENTIDAD) {
                    hay_zombie = 1;
                    break;
                }
            }
            if (!hay_zombie) {
                continue;
            }

            int siguiente = j + 1;
            if (siguiente >= juego->columnas) {
                continue;
            }

            // Si el zombie esta justo enfrente, impacta inmediatamente
            if (juego->tipo_entidad[i][siguiente] == ZOMBIE_ENTIDAD) {
                juego->vida[i][siguiente]--;
                if (juego->vida[i][siguiente] <= 0) {
                    juego->tipo_entidad[i][siguiente] = VACIO;
                    juego->clase_entidad[i][siguiente] = PASTO;
                    juego->vida[i][siguiente] = 0;
                }
                juego->ultimo_evento[i][j] = tiempo_actual;
            } else if (juego->proyectiles[i][siguiente] == 0) {
                // Puede aparecer aunque en 'siguiente' haya otra planta (girasol, nuez u otro lanzaguisantes)
                juego->proyectiles[i][siguiente] = 1;
                juego->ultimo_proyectil[i][siguiente] = tiempo_ms;
                juego->ultimo_evento[i][j] = tiempo_actual;
            }
        }
    }
}

// Avanza proyectiles hacia la derecha cada COOLDOWN_PROYECTIL_MS milisegundos,
// atravesando plantas amigas hasta golpear un zombie o salir del tablero.
void mover_proyectiles(EstadoJuego *juego, uint64_t tiempo_ms) {
    if (juego == NULL) {
        return;
    }

    for (int i = 0; i < juego->filas; i++) {
        for (int j = juego->columnas - 1; j >= 0; j--) {
            if (juego->proyectiles[i][j] == 0) {
                continue;
            }
            if (tiempo_ms - juego->ultimo_proyectil[i][j] < COOLDOWN_PROYECTIL_MS) {
                continue;
            }

            juego->proyectiles[i][j] = 0;
            juego->ultimo_proyectil[i][j] = 0;

            int siguiente = j + 1;
            if (siguiente >= juego->columnas) {
                continue;
            }

            if (juego->tipo_entidad[i][siguiente] == ZOMBIE_ENTIDAD) {
                juego->vida[i][siguiente]--;
                if (juego->vida[i][siguiente] <= 0) {
                    juego->tipo_entidad[i][siguiente] = VACIO;
                    juego->clase_entidad[i][siguiente] = PASTO;
                    juego->vida[i][siguiente] = 0;
                }
            } else {
                // Avanza tanto sobre VACIO como sobre PLANTA (las plantas dejan pasar guisantes)
                juego->proyectiles[i][siguiente] = 1;
                juego->ultimo_proyectil[i][siguiente] = tiempo_ms;
            }
        }
    }
}

// Indica si queda al menos un zombie vivo en el tablero
int quedan_zombies_en_mapa(const EstadoJuego *juego) {
    if (juego == NULL || juego->tipo_entidad == NULL) {
        return 0;
    }

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            if (juego->tipo_entidad[i][j] == ZOMBIE_ENTIDAD) {
                return 1;
            }
        }
    }
    return 0;
}

// Controla oleadas finitas, descansos y aparicion de zombies cada INTERVALO_SPAWN_ZOMBIE s
void generar_oleada(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego == NULL || juego->juego_activo == 0) {
        return;
    }
    if (juego->filas <= 0 || juego->columnas <= 0) {
        return;
    }

    if (juego->max_oleadas <= 0) {
        juego->max_oleadas = (oleadas > 0) ? oleadas : 3;
    }
    if (juego->zombies_objetivo_oleada <= 0) {
        juego->zombies_objetivo_oleada = zombies_para_oleada(juego->oleada_actual);
    }

    if (juego->en_descanso) {
        if (tiempo_actual - juego->fin_descanso >= TIEMPO_DESCANSO_OLEADA) {
            juego->en_descanso = 0;
            juego->oleada_actual++;
            juego->zombies_generados_oleada = 0;
            juego->zombies_objetivo_oleada = zombies_para_oleada(juego->oleada_actual);
            juego->ultimo_spawn = tiempo_actual;
        }
        return;
    }

    if (juego->zombies_generados_oleada < juego->zombies_objetivo_oleada) {
        if (tiempo_actual - juego->ultimo_spawn >= INTERVALO_SPAWN_ZOMBIE) {
            int fila_inicio = rand() % juego->filas;
            int columna_final = juego->columnas - 1;
            int vida_z = (vida_Zombies > 0) ? vida_Zombies : 3;

            for (int offset = 0; offset < juego->filas; offset++) {
                int fila = (fila_inicio + offset) % juego->filas;
                if (juego->tipo_entidad[fila][columna_final] == VACIO) {
                    int clase_z = (juego->zombies_generados_oleada % 3 == 2) ? FLAG_ZOMBIE : ZOMBIE_NORMAL;
                    juego->tipo_entidad[fila][columna_final] = ZOMBIE_ENTIDAD;
                    juego->clase_entidad[fila][columna_final] = clase_z;
                    juego->vida[fila][columna_final] = vida_z;
                    juego->ultimo_mov_zombie[fila][columna_final] = tiempo_actual;
                    juego->ultimo_evento[fila][columna_final] = tiempo_actual;
                    juego->zombies_generados_oleada++;
                    break;
                }
            }
            juego->ultimo_spawn = tiempo_actual;
        }
        return;
    }

    if (!quedan_zombies_en_mapa(juego)) {
        if (juego->oleada_actual >= juego->max_oleadas) {
            juego->victoria = 1;
            juego->juego_activo = 0;
        } else {
            juego->en_descanso = 1;
            juego->fin_descanso = tiempo_actual;
        }
    }
}

// Mueve zombies hacia la izquierda o muerde plantas/nuez cada COOLDOWN_ZOMBIE segundos
void mover_zombies(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego == NULL || juego->juego_activo == 0) {
        return;
    }

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            if (juego->tipo_entidad[i][j] != ZOMBIE_ENTIDAD) {
                continue;
            }

            if (tiempo_actual - juego->ultimo_mov_zombie[i][j] < COOLDOWN_ZOMBIE) {
                continue;
            }

            if (j == 0) {
                juego->vida_casa -= DANO_CASA;
                juego->tipo_entidad[i][j] = VACIO;
                juego->clase_entidad[i][j] = PASTO;
                juego->vida[i][j] = 0;
                juego->ultimo_mov_zombie[i][j] = tiempo_actual;
            } else if (juego->tipo_entidad[i][j - 1] == VACIO) {
                juego->tipo_entidad[i][j - 1] = ZOMBIE_ENTIDAD;
                juego->clase_entidad[i][j - 1] = juego->clase_entidad[i][j];
                juego->vida[i][j - 1] = juego->vida[i][j];
                juego->ultimo_mov_zombie[i][j - 1] = tiempo_actual;
                juego->ultimo_evento[i][j - 1] = tiempo_actual;

                juego->tipo_entidad[i][j] = VACIO;
                juego->clase_entidad[i][j] = PASTO;
                juego->vida[i][j] = 0;
                juego->ultimo_mov_zombie[i][j] = 0;
            } else if (juego->tipo_entidad[i][j - 1] == PLANTA) {
                int danio = (juego->clase_entidad[i][j - 1] == NUEZ) ? DANO_NUEZ : DANO_PLANTA_NORMAL;
                juego->vida[i][j - 1] -= danio;
                juego->ultimo_mov_zombie[i][j] = tiempo_actual;

                if (juego->vida[i][j - 1] <= 0) {
                    juego->tipo_entidad[i][j - 1] = VACIO;
                    juego->clase_entidad[i][j - 1] = PASTO;
                    juego->vida[i][j - 1] = 0;
                }
            }
        }
    }
}

// Revisa si la casa fue destruida
void comprobar_fin_juego(EstadoJuego *juego) {
    if (juego == NULL) {
        return;
    }

    if (juego->vida_casa <= 0) {
        juego->vida_casa = 0;
        juego->juego_activo = 0;
        juego->victoria = 0;
    }
}

// Avanza un paso completo de la logica del juego
void actualizar_juego(EstadoJuego *juego, time_t tiempo_actual, uint64_t tiempo_ms) {
    if (juego == NULL || juego->juego_activo == 0) {
        return;
    }

    generar_soles(juego, tiempo_actual);
    actualizar_girasoles(juego, tiempo_actual);
    actualizar_lanzaguisantes(juego, tiempo_actual, tiempo_ms);
    mover_proyectiles(juego, tiempo_ms);
    generar_oleada(juego, tiempo_actual);
    mover_zombies(juego, tiempo_actual);
    comprobar_fin_juego(juego);
}

// Guarda el estado actual de la partida en un archivo de texto
int guardar_partida(const EstadoJuego *juego, const char *ruta_archivo) {
    if (juego == NULL || ruta_archivo == NULL || juego->clase_entidad == NULL || juego->vida == NULL) {
        return 0;
    }

    FILE *archivo = fopen(ruta_archivo, "w");
    if (archivo == NULL) {
        return 0;
    }

    fprintf(archivo, "%d %d\n", juego->filas, juego->columnas);
    fprintf(archivo, "%d %d %d %d %d %d %d %d %d %d\n",
            juego->soles,
            juego->vida_casa,
            juego->oleada_actual,
            juego->max_oleadas,
            juego->zombies_generados_oleada,
            juego->zombies_objetivo_oleada,
            juego->en_descanso,
            juego->frecuencia_disparo,
            juego->frecuencia_soles,
            juego->juego_activo);
    fprintf(archivo, "%d\n", juego->victoria);

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            fprintf(archivo, "%d ", juego->clase_entidad[i][j]);
        }
        fprintf(archivo, "\n");
    }

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            fprintf(archivo, "%d ", juego->vida[i][j]);
        }
        fprintf(archivo, "\n");
    }

    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {
            int p = (juego->proyectiles != NULL) ? juego->proyectiles[i][j] : 0;
            fprintf(archivo, "%d ", p);
        }
        fprintf(archivo, "\n");
    }

    fclose(archivo);
    return 1;
}

// Carga una partida guardada desde archivo y reconstruye las matrices dinamicas
int cargar_partida(EstadoJuego *juego, const char *ruta_archivo) {
    if (juego == NULL || ruta_archivo == NULL) {
        return 0;
    }

    FILE *archivo = fopen(ruta_archivo, "r");
    if (archivo == NULL) {
        return 0;
    }

    int filas = 0;
    int columnas = 0;
    if (fscanf(archivo, "%d %d", &filas, &columnas) != 2 || filas <= 0 || columnas <= 0) {
        fclose(archivo);
        return 0;
    }

    EstadoJuego cargada;
    poner_a_cero(&cargada);
    cargada.filas = filas;
    cargada.columnas = columnas;

    if (fscanf(archivo, "%d %d %d %d %d %d %d %d %d %d %d",
               &cargada.soles,
               &cargada.vida_casa,
               &cargada.oleada_actual,
               &cargada.max_oleadas,
               &cargada.zombies_generados_oleada,
               &cargada.zombies_objetivo_oleada,
               &cargada.en_descanso,
               &cargada.frecuencia_disparo,
               &cargada.frecuencia_soles,
               &cargada.juego_activo,
               &cargada.victoria) != 11) {
        fclose(archivo);
        return 0;
    }

    cargada.tipo_entidad = crear_matriz(filas, columnas);
    cargada.clase_entidad = crear_matriz(filas, columnas);
    cargada.vida = crear_matriz(filas, columnas);
    cargada.proyectiles = crear_matriz(filas, columnas);
    cargada.ultimo_evento = crear_matriz_tiempo(filas, columnas);
    cargada.ultimo_proyectil = crear_matriz_ms(filas, columnas);
    cargada.ultimo_mov_zombie = crear_matriz_tiempo(filas, columnas);

    if (cargada.tipo_entidad == NULL ||
        cargada.clase_entidad == NULL ||
        cargada.vida == NULL ||
        cargada.proyectiles == NULL ||
        cargada.ultimo_evento == NULL ||
        cargada.ultimo_proyectil == NULL ||
        cargada.ultimo_mov_zombie == NULL) {
        liberar_juego(&cargada);
        fclose(archivo);
        return 0;
    }

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            if (fscanf(archivo, "%d", &cargada.clase_entidad[i][j]) != 1) {
                liberar_juego(&cargada);
                fclose(archivo);
                return 0;
            }
        }
    }

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            if (fscanf(archivo, "%d", &cargada.vida[i][j]) != 1) {
                liberar_juego(&cargada);
                fclose(archivo);
                return 0;
            }
        }
    }

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            if (fscanf(archivo, "%d", &cargada.proyectiles[i][j]) != 1) {
                cargada.proyectiles[i][j] = 0;
            }
        }
    }

    fclose(archivo);

    time_t ahora = time(NULL);
    int vida_z = (vida_Zombies > 0) ? vida_Zombies : 3;
    sincronizar_entidades_desde_clase(&cargada, vida_z, ahora);

    if (cargada.frecuencia_disparo <= 0) {
        cargada.frecuencia_disparo = (frecuencia_Disparo_Plantas > 0) ? frecuencia_Disparo_Plantas : 2;
    }
    if (cargada.frecuencia_soles <= 0) {
        cargada.frecuencia_soles = (frecuencia_Soles > 0) ? frecuencia_Soles : 5;
    }
    if (cargada.max_oleadas <= 0) {
        cargada.max_oleadas = (oleadas > 0) ? oleadas : 3;
    }
    if (cargada.oleada_actual <= 0) {
        cargada.oleada_actual = 1;
    }
    if (cargada.zombies_objetivo_oleada <= 0) {
        cargada.zombies_objetivo_oleada = zombies_para_oleada(cargada.oleada_actual);
    }

    cargada.ultimo_solar = ahora;
    cargada.ultimo_spawn = ahora;
    cargada.fin_descanso = ahora;

    liberar_juego(juego);
    *juego = cargada;
    return 1;
}

// Libera todas las matrices reservadas en EstadoJuego
void liberar_juego(EstadoJuego *juego) {
    if (juego == NULL) {
        return;
    }

    liberar_matriz(juego->clase_entidad, juego->filas);
    liberar_matriz(juego->tipo_entidad, juego->filas);
    liberar_matriz(juego->vida, juego->filas);
    liberar_matriz(juego->proyectiles, juego->filas);
    liberar_matriz_tiempo(juego->ultimo_evento, juego->filas);
    liberar_matriz_ms(juego->ultimo_proyectil, juego->filas);
    liberar_matriz_tiempo(juego->ultimo_mov_zombie, juego->filas);

    poner_a_cero(juego);
}