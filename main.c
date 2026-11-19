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
        printf("Elija que mapa quiere cargar. 1 (predeterminado) o 2 (aleatorio):\n");
        scanf("%d", &elegir);
    } while (elegir < 1 || elegir > 2);



    if(elegir == 1){ // cargar el mapa predeterminado
        mapa = cargar_mapa("mapa.txt", &filas, &columnas);
    } else { // cargar un mapa al azar
        do{
            printf("Ingrese las filas y columnas para el mapa al azar (minimo 3x3): \n");
            scanf("%d %d", &filas, &columnas);
        } while (filas < 3 || columnas < 3);
        generar_mapa_aleatorio(&filas, &columnas);
        mapa = cargar_mapa("mapa_aleatorio.txt", &filas, &columnas);
    }
    
    leer_config();
    

    EstadoJuego juego;
    inicializar_juego(&juego, mapa, filas, columnas);

    int eleccion = 0, AuxFila, AuxColumna, zombies_vivos, exito;

    while(juego.juego_activo){

        if(juego.vida_casa <= 0){
            printf("Los Zombies Ganaron!!!\n");
            break;
        }

        printf("Cant. de soles: %d \nCant. de Vida de la casa: %d\n\n", juego.soles, juego.vida_casa);
        for(int i = 0; i < filas; i++){
            printf("    ");
            for(int j = 0; j < columnas; j++){
                if(juego.proyectiles[i][j] == 1){
                    printf("*");
                    // para los gisantes(que son como balas)
                } else {
                    // Aqui imprimimos segun que este en la matriz

                    // 0 = espacio vacio
                    // 1 = sol
                    // 2 = Lanza gusantes
                    // 3 = Nuez
                    // 4 = zombie normal
                    // 5 = zombie rapido

                    if(juego.clase_entidad[i][j] == 0){ // .
                        printf(".");
                    } else if(juego.clase_entidad[i][j] == 1){
                        printf("S");
                    } else if(juego.clase_entidad[i][j] == 2){
                        printf("L");
                    } else if(juego.clase_entidad[i][j] == 3){
                        printf("N");
                    } else if(juego.clase_entidad[i][j] == 4){ // zombie normal
                        printf("z");
                    } else if(juego.clase_entidad[i][j] == 5){ // zombie rapido
                        printf("Z");
                    }
                }
            }
            printf("\n");
        }
        printf("\n");

        zombies_vivos = 0;
        for(int i = 0; i < filas; i++){
            for(int j = 0; j < columnas; j++){
                if((juego.clase_entidad[i][j] == ZOMBIE_NORMAL) || (juego.clase_entidad[i][j] == FLAG_ZOMBIE)){
                    zombies_vivos++;
                }
            }
        }
        if(zombies_vivos == 0){
            printf("\nYa no hay zombies\nHaz ganado!!! :D\n");
            break;
        }

        printf("Elige una opción:\n");
        printf("(1) Plantar Girasol (%d soles)\n", COSTO_GIRASOL);
        printf("(2) Plantar Lanzaguisantes (%d soles)\n", COSTO_LANZAGUISANTES);
        printf("(3) Plantar Nuez (%d soles)\n", COSTO_NUEZ);
        printf("(4) Pasar turno\n");
        scanf("%d", &eleccion);

        if(eleccion != 4){
            printf("Ingrese la fila y la columna (EN ESE ORDEN):\n");
            scanf("%d %d", &AuxFila, &AuxColumna);
            exito = colocar_planta(&juego, eleccion, AuxFila, AuxColumna);
            if(exito){ // si se logro plantar al, el valor va a ser diferente de 0
                printf("¡Planta colocada!\n");
            } else {
                printf("No se pudo colocar (casilla ocupada o faltan soles).\n");
            }
        }

        actualizar_juego(&juego, time(NULL));

        // Para que no sea todo seguido
        printf("Presiona Enter para el siguiente turno\n");
        getchar();
        getchar();
        // son dos, uno para sacar el salto de linea basura 
        // y el otro para sacar el salto de linea hecho para pasar al siguiente turno.
    }

    


    // Liberar la memoria ocupada por el mapa
    liberar_juego(&juego);
    
    return 0;
}