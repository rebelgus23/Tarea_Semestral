// gcc main.c tarea1.c -o juego
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tarea1.h"

int main(){
    srand(time(NULL));
    int filas, columnas, elegir;

    do{
        printf("Elija que mapa quiere cargar. 1 (predeterminado) o 2 (aleatorio)");
        scanf("%d %d", &filas, &columnas);
    } while (elegir < 1 || elegir > 2);

    if(elegir == 1){ // cargar el mapa predeterminado
        int **mapa = cargar_mapa("mapa_aleatorio.txt", &filas, &columnas);
    } else { // cargar un mapa al azar
        do{
            printf("Ingrese las filas y columnas para el mapa al azar (minimo 3x3): ");
            scanf("%d %d", &filas, &columnas);
        } while (filas < 3 || columnas < 3);

        // En la siguiente funcion podriamos hacer elegir entre un modo normal y dificil
        // Lo que los diferenciaria seria la aparicion de las cosas, 
        // ya que actualmente estan configuradas. Podriamos dejar que queden al azar

        generar_mapa_aleatorio(&filas, &columnas);
    }
    
    leer_config();
    


    // Liberar la memoria ocupada por el mapa
    for (int i = 0; i < filas; i++) {
        free(mapa[i]);
    }
    free(mapa);
    
    return 0;
}