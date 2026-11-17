// gcc main.c tarea1.c -o juego
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tarea1.h"
#include "logica.h"

int main(){
    srand(time(NULL));
    int filas, columnas, elegir;
    int **mapa = NULL;

    do{
        printf("Elija que mapa quiere cargar. 1 (predeterminado) o 2 (aleatorio)\n");
        scanf("%d", &elegir);
    } while (elegir < 1 || elegir > 2);

    if(elegir == 1){ // cargar el mapa predeterminado
        mapa = cargar_mapa("mapa.txt", &filas, &columnas);
    } else { // cargar un mapa al azar
        do{
            printf("Ingrese las filas y columnas para el mapa al azar (minimo 3x3): ");
            scanf("%d %d", &filas, &columnas);
        } while (filas < 3 || columnas < 3);

        // En la siguiente funcion podriamos hacer elegir entre un modo normal y dificil
        // Lo que los diferenciaria seria la aparicion de las cosas, 
        // ya que actualmente estan configuradas. Podriamos dejar que queden al azar

        generar_mapa_aleatorio(&filas, &columnas);
        mapa = cargar_mapa("mapa_aleatorio.txt", &filas, &columnas);
    }
    
    leer_config();

    EstadoJuego mi_juego;
    
    // Inicializamos el juego pasándole el mapa y las dimensiones
    if (inicializar_juego(&mi_juego, mapa, filas, columnas)) {
        printf("\n¡Juego inicializado con exito!\n");
        printf("Soles: %d | Vida Casa: %d\n", mi_juego.soles, mi_juego.vida_casa);
        
        // Probemos colocar una planta (Girasol) en la posicion 0,0
        // (Recuerda que el mapa debe tener un 0 en esa posición para que funcione)
        if (colocar_planta(&mi_juego, GIRASOL, 0, 0)) {
            printf("Girasol colocado en 0,0. Soles restantes: %d\n", mi_juego.soles);
        } else {
            printf("No se pudo colocar el Girasol (quizas no habia pasto o faltaban soles).\n");
        }

        // Imprimimos el mapa usando la estructura del juego
        printf("\n--- MAPA ACTUAL (Usando EstadoJuego) ---\n");
        for (int i = 0; i < mi_juego.filas; i++) {
            for (int j = 0; j < mi_juego.columnas; j++) {
                printf("%d ", mi_juego.mapa[i][j]);
            }
            printf("\n");
        }
        
        // Aquí a futuro tus compañeras implementarán un ciclo while con actualizar_juego()
        
    } else {
        printf("Error al inicializar el juego.\n");
    }

    // Nota: Si tus compañeras no hicieron la funcion liberar_juego en logica.c, 
    // esto podría dar error. Si da error, tendrás que liberar las matrices a mano.
    // liberar_juego(&mi_juego); 
    


    // Liberar la memoria ocupada por el mapa
    for (int i = 0; i < filas; i++) {
        free(mapa[i]);
    }
    free(mapa);
    
    return 0;
}