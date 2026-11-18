#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "logica.h"
#include "tarea1.h"


// Crea una matriz dinamica inicializada en cero
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


// Reserva memoria para almacenar los tiempos de cada casilla
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


// Libera una matriz de enteros
void liberar_matriz(int **matriz, int filas) {
    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < filas; i++) {
        free(matriz[i]);
    }

    free(matriz);
}


// Libera una matriz que almacena tiempos
void liberar_matriz_tiempo(time_t **matriz, int filas) {
    if (matriz == NULL) {
        return;
    }

    for (int i = 0; i < filas; i++) {
        free(matriz[i]);
    }

    free(matriz);
}


// Inicializa las matrices y los valores iniciales de la partida
int inicializar_juego(EstadoJuego *juego, int **mapa, int filas, int columnas) {
    if (juego == NULL || mapa == NULL || filas <= 0 || columnas <= 0 ||
    oleadas <= 0 || vida_Zombies <= 0 || vida_Casa <= 0) {
    return 0;
}

    // Se comprueba que los valores del mapa sean validos
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

    juego->filas = filas;
    juego->columnas = columnas;

    // Se inicializan los punteros
    juego->tipo_entidad = NULL;
    juego->clase_entidad = NULL;
    juego->vida = NULL;
    juego->proyectiles = NULL;
    juego->ultimo_evento = NULL;
    juego->ultimo_proyectil = NULL;

    // Se reservan las matrices necesarias
    juego->tipo_entidad = crear_matriz(filas, columnas);
    juego->vida = crear_matriz(filas, columnas);
    juego->proyectiles = crear_matriz(filas, columnas);

    juego->ultimo_evento = crear_matriz_tiempo(filas, columnas);
    juego->ultimo_proyectil = crear_matriz_tiempo(filas, columnas);

    if (juego->tipo_entidad == NULL || juego->vida == NULL ||
        juego->proyectiles == NULL || juego->ultimo_evento == NULL ||
        juego->ultimo_proyectil == NULL) {

        liberar_matriz(juego->tipo_entidad, filas);
        liberar_matriz(juego->vida, filas);
        liberar_matriz(juego->proyectiles, filas);
        liberar_matriz_tiempo(juego->ultimo_evento, filas);
        liberar_matriz_tiempo(juego->ultimo_proyectil, filas);

        juego->tipo_entidad = NULL;
        juego->vida = NULL;
        juego->proyectiles = NULL;
        juego->ultimo_evento = NULL;
        juego->ultimo_proyectil = NULL;

        return 0;
    }

    // El estado del juego toma el control del mapa recibido
    juego->clase_entidad = mapa;

    time_t tiempo_inicio = time(NULL);

    // Clasifica cada casilla y asigna la vida correspondiente
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {

            int tipo = mapa[i][j];

            if (tipo == PASTO) {
                juego->tipo_entidad[i][j] = VACIO;
                juego->vida[i][j] = 0;
            }
            else if (tipo == GIRASOL || tipo == LANZAGUISANTES) {
                juego->tipo_entidad[i][j] = PLANTA;
                juego->vida[i][j] = 1;
            }
            else if (tipo == NUEZ) {
                juego->tipo_entidad[i][j] = PLANTA;
                juego->vida[i][j] = VIDA_NUEZ;
            }
            else if (tipo == ZOMBIE_NORMAL || tipo == FLAG_ZOMBIE) {
                juego->tipo_entidad[i][j] = ZOMBIE_ENTIDAD;
                juego->vida[i][j] = vida_Zombies;
            }

            // Se inicializan los tiempos de cada casilla
            juego->ultimo_evento[i][j] = tiempo_inicio;
            juego->ultimo_proyectil[i][j] = tiempo_inicio;
        }
    }

    juego->soles = 50;
    juego->vida_casa = vida_Casa;
    juego->oleada_actual = 0;

    juego->frecuencia_disparo = frecuencia_Disparo_Plantas;
    juego->frecuencia_soles = frecuencia_Soles;

    juego->juego_activo = 1;
    juego->victoria = 0;

    juego->ultimo_solar = tiempo_inicio;

    // Se inicializan los valores de las oleadas
    juego->estado_oleada = PREPARACION;
    juego->zombies_por_generar = 0;
    juego->zombies_generados = 0;

    juego->inicio_partida = tiempo_inicio;
    juego->ultimo_spawn = tiempo_inicio;
    juego->inicio_descanso = tiempo_inicio;

    return 1;
}


// Comprueba si se puede colocar una planta y descuenta su costo
int colocar_planta(EstadoJuego *juego, int tipo, int fila, int columna) {
    if (juego == NULL || juego->juego_activo == 0 ||
    fila < 0 || fila >= juego->filas ||
    columna < 0 || columna >= juego->columnas) {
    return 0;
}

    if (juego->tipo_entidad[fila][columna] != VACIO) {
        return 0;
    }

    int costo;
    int vida;

    if (tipo == GIRASOL) {
        costo = COSTO_GIRASOL;
        vida = 1;
    }
    else if (tipo == LANZAGUISANTES) {
        costo = COSTO_LANZAGUISANTES;
        vida = 1;
    }
    else if (tipo == NUEZ) {
        costo = COSTO_NUEZ;
        vida = VIDA_NUEZ;
    }
    else {
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


// Genera soles de forma periodica
void generar_soles(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego->frecuencia_soles <= 0) {
        return;
    }

    if (tiempo_actual - juego->ultimo_solar >= juego->frecuencia_soles) {
        juego->soles += SOLES_GENERADOS;
        juego->ultimo_solar = tiempo_actual;
    }
}


// Controla la generacion de soles de los girasoles
void actualizar_girasoles(EstadoJuego *juego, time_t tiempo_actual) {
    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {

            if (juego->clase_entidad[i][j] == GIRASOL &&
                juego->frecuencia_soles > 0) {

                if (tiempo_actual - juego->ultimo_evento[i][j] >= juego->frecuencia_soles) {
                    juego->soles += SOLES_GENERADOS;
                    juego->ultimo_evento[i][j] = tiempo_actual;
                }
            }
        }
    }
}


// Comprueba si los lanzaguisantes pueden disparar
void actualizar_lanzaguisantes(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego->frecuencia_disparo <= 0) {
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

            // Solo dispara si encuentra un zombie en su misma fila
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

            if (juego->tipo_entidad[i][siguiente] == ZOMBIE_ENTIDAD) {
                juego->vida[i][siguiente]--;

                if (juego->vida[i][siguiente] <= 0) {
                    juego->tipo_entidad[i][siguiente] = VACIO;
                    juego->clase_entidad[i][siguiente] = PASTO;
                    juego->vida[i][siguiente] = 0;
                }

                juego->ultimo_evento[i][j] = tiempo_actual;
            }
            else if (juego->tipo_entidad[i][siguiente] == VACIO &&
                     juego->proyectiles[i][siguiente] == 0) {

                juego->proyectiles[i][siguiente] = 1;
                juego->ultimo_proyectil[i][siguiente] = tiempo_actual;

                juego->ultimo_evento[i][j] = tiempo_actual;
            }
        }
    }
}


// Avanza los proyectiles y comprueba si impactan contra un zombie
void mover_proyectiles(EstadoJuego *juego, time_t tiempo_actual) {
    for (int i = 0; i < juego->filas; i++) {

        // Se recorre desde la derecha para evitar mover un proyectil dos veces
        for (int j = juego->columnas - 1; j >= 0; j--) {

            if (juego->proyectiles[i][j] == 0) {
                continue;
            }

            if (tiempo_actual - juego->ultimo_proyectil[i][j] < 1) {
                continue;
            }

            juego->proyectiles[i][j] = 0;

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
            }
            else if (juego->tipo_entidad[i][siguiente] == VACIO &&
                     juego->proyectiles[i][siguiente] == 0) {

                juego->proyectiles[i][siguiente] = 1;
                juego->ultimo_proyectil[i][siguiente] = tiempo_actual;
            }
        }
    }
}


// Controla el movimiento y los ataques de los zombies
void actualizar_zombies(EstadoJuego *juego, time_t tiempo_actual) {

    // reccorren de izquierda a derecha
    for (int i = 0; i < juego->filas; i++) {
        for (int j = 0; j < juego->columnas; j++) {

            if (juego->tipo_entidad[i][j] != ZOMBIE_ENTIDAD) {
                continue;
            }

            int frecuencia;

            // Se determina la velocidad segun el tipo de zombie
            if (juego->clase_entidad[i][j] == ZOMBIE_NORMAL) {
                frecuencia = FRECUENCIA_ZOMBIE_NORMAL;
            }
            else {
                frecuencia = FRECUENCIA_FLAG_ZOMBIE;
            }

            // Se comprueba si debe atacar una planta o la casa
            if (j == 0) {
                frecuencia = FRECUENCIA_ATAQUE_ZOMBIE;
            }
            else if (juego->tipo_entidad[i][j - 1] == PLANTA) {
                frecuencia = FRECUENCIA_ATAQUE_ZOMBIE;
            }

            // Se comprueba si ya paso el tiempo necesario
            if (tiempo_actual - juego->ultimo_evento[i][j] < frecuencia) {
                continue;
            }

            // Si el zombie llega al extremo izquierdo, ataca  ala casa
            if (j == 0) {

                juego->vida_casa -= DANO_ZOMBIE;
                juego->ultimo_evento[i][j] = tiempo_actual;

            }

            // Si encuentra una planta, la ataca
            else if (juego->tipo_entidad[i][j - 1] == PLANTA) {

                juego->vida[i][j - 1] -= DANO_ZOMBIE;

                // Se elimina la planta si pierde toda su vida
                if (juego->vida[i][j - 1] <= 0) {

                    juego->tipo_entidad[i][j - 1] = VACIO;
                    juego->clase_entidad[i][j - 1] = PASTO;
                    juego->vida[i][j - 1] = 0;
                }

                juego->ultimo_evento[i][j] = tiempo_actual;

            }

            // Si encuentra una casilla vacia, avanza
            else if (juego->tipo_entidad[i][j - 1] == VACIO) {

                juego->tipo_entidad[i][j - 1] = ZOMBIE_ENTIDAD;
                juego->clase_entidad[i][j - 1] = juego->clase_entidad[i][j];
                juego->vida[i][j - 1] = juego->vida[i][j];

                juego->ultimo_evento[i][j - 1] = tiempo_actual;

                // Se libera la casilla anterior
                juego->tipo_entidad[i][j] = VACIO;
                juego->clase_entidad[i][j] = PASTO;
                juego->vida[i][j] = 0;
            }
        }
    }
}


// Comprueba si quedan zombies en el jardin
// Comprueba si quedan zombies en el jardin
int quedan_zombies(EstadoJuego *juego) {

    if (juego == NULL) {
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


// Genera un zombie en el extremo derecho del mapa
int generar_zombie(EstadoJuego *juego, time_t tiempo_actual) {

    if (juego == NULL || juego->filas <= 0 || juego->columnas <= 0) {
    return 0;
}

    int columna = juego->columnas - 1;
    int fila_inicial = rand() % juego->filas;

    // Se buscan casillas disponibles en la ultima columna
    for (int i = 0; i < juego->filas; i++) {

        int fila = (fila_inicial + i) % juego->filas;

        if (juego->tipo_entidad[fila][columna] == VACIO) {

            int tipo;

            // Se genera un flag zombie cada tres apariciones
            if (juego->zombies_generados % 3 == 2) {
                tipo = FLAG_ZOMBIE;
            }
            else {
                tipo = ZOMBIE_NORMAL;
            }

            juego->tipo_entidad[fila][columna] = ZOMBIE_ENTIDAD;
            juego->clase_entidad[fila][columna] = tipo;
            juego->vida[fila][columna] = vida_Zombies;

            juego->ultimo_evento[fila][columna] = tiempo_actual;

            juego->zombies_generados++;

            return 1;
        }
    }

    return 0;
}


// Controla la aparicion y el avance de las oleadas
void actualizar_oleadas(EstadoJuego *juego, time_t tiempo_actual) {

    if (juego == NULL || !juego->juego_activo) {
        return;
    }

    // Se controla el tiempo de preparacion inicial
    if (juego->estado_oleada == PREPARACION) {

        if (tiempo_actual - juego->inicio_partida >= TIEMPO_PREPARACION) {

            juego->oleada_actual = 1;
            juego->zombies_por_generar = ZOMBIES_INICIALES;
            juego->zombies_generados = 0;

            juego->ultimo_spawn = tiempo_actual;
            juego->estado_oleada = OLEADA;
        }

        return;
    }

    // Se controla la aparicion de zombies durante una oleada
    if (juego->estado_oleada == OLEADA) {

        if (juego->zombies_generados < juego->zombies_por_generar) {

            if (tiempo_actual - juego->ultimo_spawn >= INTERVALO_APARICION) {

                if (generar_zombie(juego, tiempo_actual)) {
                    juego->ultimo_spawn = tiempo_actual;
                }
            }
        }

        // Se comprueba si aparecieron todos los zombies y ya no quedan en el jardin
        if (juego->zombies_generados == juego->zombies_por_generar &&
            !quedan_zombies(juego)) {

            if (juego->oleada_actual >= oleadas) {

                juego->estado_oleada = PARTIDA_TERMINADA;
                juego->juego_activo = 0;
                juego->victoria = 1;

            }
            else {

                juego->estado_oleada = DESCANSO;
                juego->inicio_descanso = tiempo_actual;
            }
        }

        return;
    }

    // Se controla el descanso entre oleadas
    if (juego->estado_oleada == DESCANSO) {

        if (tiempo_actual - juego->inicio_descanso >= TIEMPO_DESCANSO) {

            juego->oleada_actual++;

            // Cada oleada aumenta la cantidad de zombies
            juego->zombies_por_generar += ZOMBIES_ADICIONALES;
            juego->zombies_generados = 0;

            juego->ultimo_spawn = tiempo_actual;
            juego->estado_oleada = OLEADA;
        }

        return;
    }
}


// Comprueba si se cumplen las condiciones para terminar la partida
void comprobar_fin_juego(EstadoJuego *juego) {

    if (juego == NULL) {
        return;
    }

    // Se comprueba si la casa fue destruida
    if (juego->vida_casa <= 0) {

        juego->vida_casa = 0;
        juego->juego_activo = 0;
        juego->victoria = 0;
        juego->estado_oleada = PARTIDA_TERMINADA;
    }
}


// Ejecuta las actualizaciones correspondientes al estado del juego
// Ejecuta las actualizaciones correspondientes al estado del juego
void actualizar_juego(EstadoJuego *juego, time_t tiempo_actual) {
    if (juego == NULL || juego->juego_activo == 0) {
        return;
    }

    generar_soles(juego, tiempo_actual);

    actualizar_girasoles(juego, tiempo_actual);

    actualizar_oleadas(juego, tiempo_actual);

    // Se comprueba si las oleadas terminaron la partida
    if (juego->juego_activo == 0) {
        return;
    }

    actualizar_lanzaguisantes(juego, tiempo_actual);

    mover_proyectiles(juego, tiempo_actual);

    actualizar_zombies(juego, tiempo_actual);

    comprobar_fin_juego(juego);
}

// Libera la memoria utilizada por el estado del juego
void liberar_juego(EstadoJuego *juego) {
    if (juego == NULL) {
        return;
    }

    liberar_matriz(juego->clase_entidad, juego->filas);
    liberar_matriz(juego->tipo_entidad, juego->filas);
    liberar_matriz(juego->vida, juego->filas);
    liberar_matriz(juego->proyectiles, juego->filas);

    liberar_matriz_tiempo(juego->ultimo_evento, juego->filas);
    liberar_matriz_tiempo(juego->ultimo_proyectil, juego->filas);

    juego->clase_entidad = NULL;
    juego->tipo_entidad = NULL;
    juego->vida = NULL;
    juego->proyectiles = NULL;
    juego->ultimo_evento = NULL;
    juego->ultimo_proyectil = NULL;
}
