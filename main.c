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
        printf("Elija que mapa quiere cargar. 1 (predeterminado) o 2 (aleatorio)");
        scanf("%d", &elegir);
    } while (elegir < 1 || elegir > 2);



    if(elegir == 1){ // cargar el mapa predeterminado
        mapa = cargar_mapa("mapa.txt", &filas, &columnas);
    } else { // cargar un mapa al azar
        do{
            printf("Ingrese las filas y columnas para el mapa al azar (minimo 3x3): ");
            scanf("%d %d", &filas, &columnas);
        } while (filas < 3 || columnas < 3);

        generar_mapa_aleatorio(&filas, &columnas);
    }
    
    leer_config();
    

    EstadoJuego juego;
    inicializar_juego(&juego, mapa, filas, columnas);

    int eleccion, AuxFila, AuxColumna;

    while(juego.juego_activo){
        printf("%d %d", juego.soles, juego.vida_casa);
        for(int i = 0; i < filas; i++){
            for(int j = 0; j < columnas; j++){
                if(juego.mapa[i][j] == 0){ // .
                    printf(".");
                } else if(juego.mapa[i][j] == 1){
                    printf("S");
                } else if(juego.mapa[i][j] == 2){
                    printf("L");
                } else if(juego.mapa[i][j] == 3){
                    printf("N");
                } else if(juego.mapa[i][j] == 4){ // zombie normal
                    printf("z");
                } else if(juego.mapa[i][j] == 5){ // zombie rapido
                    printf("Z");
                }
            }
            printf("\n");
        }
        printf("Elige una opción: (1) Plantar Girasol, (2) Plantar lanza gisantes, (3) Plantar nuez, (4) Pasar turno:\n");
        scanf("%d", &eleccion);

        if(eleccion != 4){
            printf("Ingrese la fila y la columna (EN ESE ORDEN):\n");
            scanf("%d %d", &AuxFila, &AuxColumna);
            colocar_planta(&juego, eleccion, AuxFila, AuxColumna);
        }

        actualizar_juego(&juego, time(NULL));
    }




    // Liberar la memoria ocupada por el mapa
    for (int i = 0; i < filas; i++) {
        free(mapa[i]);
    }
    free(mapa);
    
    return 0;
}