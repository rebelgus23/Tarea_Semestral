#include <stdio.h>
#include <stdlib.h>
#include "tarea1.h" 
#include "logica.h"

// Variables de configuracion
int oleadas;
int vida_Zombies;
int vida_Casa;
int frecuencia_Disparo_Plantas;
int frecuencia_Soles;

// Sirve para leer la configuracion que tiene que estar escrita en el archio "configuracion.txt"
void leer_config() {
    FILE *archivo = fopen("configuracion.txt", "r");
    
    if (archivo == NULL){
        fprintf(stderr, "Error: No se pudo abrir configuracion.txt\n");
    }

    fscanf(archivo, "oleadas: %d\n", &oleadas);
    fscanf(archivo, "vida_Zombies: %d\n", &vida_Zombies);
    fscanf(archivo, "vida_Casa: %d\n", &vida_Casa);
    fscanf(archivo, "frecuencia_Disparo_Plantas: %d\n", &frecuencia_Disparo_Plantas);
    fscanf(archivo, "frecuencia_Soles: %d\n", &frecuencia_Soles);

    fclose(archivo);
}

// Sirve para Poder leer un mapa precreado en un txt y copiarlo en memoria dinamica, para poder usarlo alli.
int** cargar_mapa(char* nombre, int* filas, int* columnas){
    FILE *archivo = fopen(nombre, "r");

    if(archivo == NULL){
        fprintf(stderr, "Error: No se pudo abrir %s\n", nombre);
        return NULL;
    }

    fscanf(archivo," %d %d\n", filas, columnas);

    // Mapa del jardin
    int **mapa = crear_matriz(*filas, *columnas);

    // for para leer el txt
    for(int i = 0; i < (*filas); i++){
        for(int j = 0; j < *columnas; j++){
            fscanf(archivo, " %d", &mapa[i][j]);
        }
    }

    fclose(archivo);

    return mapa;
}

// Genera un mapa aleatorio de las dimenciones dadas.
void generar_mapa_aleatorio(int* FILAS, int* COLUMNAS){
    // Me falta configurar la aparicion por tercios y diferenciar los girasoles.
    FILE *archivo = fopen("mapa_aleatorio.txt", "w");

    if(archivo == NULL){
        fprintf(stderr, "Error: No se pudo crear mapa_aleatorio.txt\n");
    }

    // 0 = espacio vacio
    // 1 = sol
    // 2 = Lanza gusantes
    // 3 = Nuez
    // 4 = zombie normal
    // 5 = zombie rapido

    int num;

    fprintf(archivo, "%d %d\n", *FILAS, *COLUMNAS); 
    for(int i = 0; i < *FILAS; i++){
        for(int j = 0; j < *COLUMNAS; j++){
            if(j < (*COLUMNAS/3)){
                fprintf(archivo, "%d ", rand() % 4); 
            } else if(j > (2*(*COLUMNAS/3))){
                fprintf(archivo, "%d ", rand() % 6); 
            } else {
                num = rand() % 6;
                if((num != 0) && (num < 4)){
                    num = 4 + rand() % 2;
                }
                fprintf(archivo, "%d ", num); 
            }
        }
        fprintf(archivo, "\n"); 
    }

    fclose(archivo);
}
