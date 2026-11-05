#include <stdio.h>
#include <stdlib.h>
#include "tarea1.h" 

// Variables de configuracion
int oleadas;
int vida_Zombies;
int vida_Casa;
int frecuencia_Disparo_Plantas;
int frecuencia_Soles;

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

int** cargar_mapa(char* nombre, int* filas, int* columnas){
    FILE *archivo = fopen(nombre, "r");

    if(archivo == NULL){
        fprintf(stderr, "Error: No se pudo abrir %s\n", nombre);
        return NULL;
    }

    fscanf(archivo," %d %d\n", filas, columnas);

    // Mapa del jardin
    int **mapa = malloc((*filas) * sizeof(int*));

    // reservar memoria para las columnas de cada fila
    for(int i = 0; i < (*filas); i++){
        mapa[i] = malloc((*columnas) * sizeof(int));
    }

    // for para leer el txt
    for(int i = 0; i < (*filas); i++){
        for(int j = 0; j < *columnas; j++){
            fscanf(archivo, " %d", &mapa[i][j]);
        }
    }

    fclose(archivo);

    return mapa;
}

void generar_mapa_aleatorio(int* FILAS, int* COLUMNAS){
    // Me falta configurar la aparicion por tercios y diferenciar los girasoles.
    FILE *archivo = fopen("mapa_aleatorio.txt", "w");

    if(archivo == NULL){
        fprintf(stderr, "Error: No se pudo crear mapa_aleatorio.txt\n");
    }

    fprintf(archivo, "%d %d\n", *FILAS, *COLUMNAS); 
    for(int i = 0; i < *FILAS; i++){
        for(int j = 0; j < *COLUMNAS; j++){
            fprintf(archivo, "%d ", rand() % 4); 
        }
        fprintf(archivo, "\n"); 
    }

    fclose(archivo);
}