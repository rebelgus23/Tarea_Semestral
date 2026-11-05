#ifndef TAREA1_H
#define TAREA1_H

    extern int oleadas;
    extern int vida_Zombies;
    extern int vida_Casa;
    extern int frecuencia_Disparo_Plantas;
    extern int frecuencia_Soles;

    // Sirve para leer las configuraciones del juego, 
    // que se encuentran en configuracion.txt
    // No devuelve nada
    void leer_config();

    // Sirve para leer el mapa preescrito, 
    // que se encuentran en mapa.txt 
    // Devuelve un puntero
    int** cargar_mapa(char* nombre, int* filas, int* columnas);

    // Genera un mapa al aleatorio
    // Devuelve un puntero
    void generar_mapa_aleatorio(int* FILAS, int* COLUMNAS);

#endif