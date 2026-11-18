/* 
Compilar en WSL:
gcc ventana_juego.c tarea1.c logica.c -o juego_grafico $(pkg-config --cflags --libs sdl3 sdl3-image)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "logica.h"
#include "tarea1.h"

// Hacemos la cuadrícula mucho más grande
#define TAM_CASILLA 90
#define ALTO_BARRA 100

SDL_Texture *cargarTextura(SDL_Renderer *renderizador, char *archivo){
    SDL_Texture *textura = IMG_LoadTexture(renderizador, archivo);
    if(textura == NULL){
        SDL_Log("Advertencia: No se encontró %s", archivo);
    }
    return textura;
}

int main(){
    srand(time(NULL));
    int filas, columnas;
    
    // 1. Cargar la lógica del juego
    int **mapa = cargar_mapa("mapa.txt", &filas, &columnas);
    if (mapa == NULL) {
        printf("Error al cargar mapa.txt\n");
        return 1;
    }
    leer_config();
    
    EstadoJuego juego;
    inicializar_juego(&juego, mapa, filas, columnas);

    // 2. Configurar SDL
    if(!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        return 1;
    }

    int ancho_base = columnas * TAM_CASILLA;
    int alto_base = (filas * TAM_CASILLA) + ALTO_BARRA;

    SDL_Window *ventana = NULL;
    SDL_Renderer *renderizador = NULL;

    if(!SDL_CreateWindowAndRenderer("Plants vs Zombies - UdeC", ancho_base, alto_base, SDL_WINDOW_RESIZABLE, &ventana, &renderizador)){
        SDL_Log("Error al crear ventana: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderizador, ancho_base, alto_base, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // 3. Cargar texturas (Si no tienes canela.jpg, usará sol.jpg)
    SDL_Texture *texturaFondo = cargarTextura(renderizador, "mapa_predeterminado.png"); 
    SDL_Texture *texturaSol = cargarTextura(renderizador, "sol.jpg");         
    SDL_Texture *texturaGirasol = cargarTextura(renderizador, "girasol.jpg"); 
    SDL_Texture *texturaLanza = cargarTextura(renderizador, "planta.png");    
    SDL_Texture *texturaNuez = cargarTextura(renderizador, "canela.jpg");     

    // Fallbacks
    if (texturaGirasol == NULL) texturaGirasol = texturaLanza;
    if (texturaNuez == NULL) texturaNuez = texturaSol; 
    
    // Ajuste de la barra superior para el nuevo tamaño
    SDL_FRect barraFondo = {0.f, 0.f, (float)ancho_base, (float)ALTO_BARRA};
    
    // Botones más grandes
    SDL_FRect btnGirasol = {10.f, 10.f, 80.f, 80.f};
    SDL_FRect btnLanza = {100.f, 10.f, 80.f, 80.f};
    SDL_FRect btnNuez = {190.f, 10.f, 80.f, 80.f};
    SDL_FRect rectSolUI = {290.f, 10.f, 80.f, 80.f}; 

    int plantaSeleccionada = 0; 
    bool jugando = true;

    // 4. Game Loop
    while(jugando){
        SDL_Event evento;
        while(SDL_PollEvent(&evento)){
            if(evento.type == SDL_EVENT_QUIT){
                jugando = false;
            }

            float mouseX, mouseY;
            SDL_RenderCoordinatesFromWindow(renderizador, evento.button.x, evento.button.y, &mouseX, &mouseY);
            SDL_FPoint pt = {mouseX, mouseY};

            // Seleccionar planta
            if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT){
                if(SDL_PointInRectFloat(&pt, &btnGirasol)) plantaSeleccionada = GIRASOL;
                else if(SDL_PointInRectFloat(&pt, &btnLanza)) plantaSeleccionada = LANZAGUISANTES;
                else if(SDL_PointInRectFloat(&pt, &btnNuez)) plantaSeleccionada = NUEZ;
            }

            // Plantar
            if(evento.type == SDL_EVENT_MOUSE_BUTTON_UP && evento.button.button == SDL_BUTTON_LEFT){
                if(plantaSeleccionada != 0 && mouseY > ALTO_BARRA){
                    int columna = (int)(mouseX / TAM_CASILLA);
                    int fila = (int)((mouseY - ALTO_BARRA) / TAM_CASILLA);

                    if(fila >= 0 && fila < filas && columna >= 0 && columna < columnas){
                        int exito = colocar_planta(&juego, plantaSeleccionada, fila, columna);
                        if(exito) {
                            printf("Planta colocada en %d, %d\n", fila, columna);
                        } else {
                            printf("No se pudo plantar (falta plata o casilla ocupada).\n");
                        }
                    }
                }
            }
        }

        // 5. Actualizar lógica
        actualizar_juego(&juego, time(NULL));

        // 6. Dibujar
        SDL_SetRenderDrawColor(renderizador, 30, 30, 30, 255);
        SDL_RenderClear(renderizador);

        // Fondo
        if(texturaFondo != NULL){
            SDL_FRect rectFondo = {0.f, (float)ALTO_BARRA, (float)ancho_base, (float)(filas * TAM_CASILLA)};
            SDL_RenderTexture(renderizador, texturaFondo, NULL, &rectFondo);
        }

        // Entidades
        for(int i = 0; i < juego.filas; i++){
            for(int j = 0; j < juego.columnas; j++){
                SDL_FRect rect = {j * TAM_CASILLA, (i * TAM_CASILLA) + ALTO_BARRA, TAM_CASILLA, TAM_CASILLA};
                int clase = juego.clase_entidad[i][j];

                // Dibujar Plantas
                if(clase == GIRASOL){
                    if(texturaGirasol) SDL_RenderTexture(renderizador, texturaGirasol, NULL, &rect);
                }
                else if(clase == LANZAGUISANTES){
                    if(texturaLanza) SDL_RenderTexture(renderizador, texturaLanza, NULL, &rect);
                }
                else if(clase == NUEZ){
                    if(texturaNuez) {
                        SDL_RenderTexture(renderizador, texturaNuez, NULL, &rect);
                        // Le ponemos un borde al perrito para que se note que es la Nuez defensiva
                        SDL_SetRenderDrawColor(renderizador, 139, 69, 19, 255); // Café oscuro
                        SDL_RenderRect(renderizador, &rect);
                    }
                }
                // Dibujar Zombies (Cuadrado rojo)
                else if(clase == ZOMBIE_NORMAL || clase == FLAG_ZOMBIE){
                    SDL_SetRenderDrawColor(renderizador, 200, 0, 0, 255);
                    SDL_RenderFillRect(renderizador, &rect);
                    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
                    SDL_RenderRect(renderizador, &rect); // Borde negro
                }

                // Dibujar Proyectiles (Guisantes)
                if(juego.proyectiles[i][j] == 1){
                    // Círculo amarillo brillante con borde negro
                    SDL_FRect rectGuisante = { (j * TAM_CASILLA) + (TAM_CASILLA/2) - 15, (i * TAM_CASILLA) + ALTO_BARRA + (TAM_CASILLA/2) - 15, 30, 30 };
                    SDL_SetRenderDrawColor(renderizador, 255, 255, 0, 255); // Amarillo
                    SDL_RenderFillRect(renderizador, &rectGuisante);
                    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255); // Borde
                    SDL_RenderRect(renderizador, &rectGuisante);
                }
            }
        }

        // 7. Dibujar UI
        SDL_SetRenderDrawColor(renderizador, 80, 80, 80, 255);
        SDL_RenderFillRect(renderizador, &barraFondo);

        // Fondos de botones
        SDL_SetRenderDrawColor(renderizador, 120, 120, 120, 255);
        SDL_RenderFillRect(renderizador, &btnGirasol);
        SDL_RenderFillRect(renderizador, &btnLanza);
        SDL_RenderFillRect(renderizador, &btnNuez);

        // Texturas de botones
        if (texturaGirasol) SDL_RenderTexture(renderizador, texturaGirasol, NULL, &btnGirasol);
        if (texturaLanza) SDL_RenderTexture(renderizador, texturaLanza, NULL, &btnLanza);
        if (texturaNuez) SDL_RenderTexture(renderizador, texturaNuez, NULL, &btnNuez);

        // Perrito (Sol)
        if (texturaSol) SDL_RenderTexture(renderizador, texturaSol, NULL, &rectSolUI);

        // Barra de soles
        SDL_SetRenderDrawColor(renderizador, 255, 215, 0, 255);
        // Hacemos la barra más ancha para que se note la cantidad
        SDL_FRect rectSoles = {380.f, 30.f, (float)(juego.soles * 2), 40.f}; 
        SDL_RenderFillRect(renderizador, &rectSoles);

        SDL_RenderPresent(renderizador);
        SDL_Delay(16); // ~60 FPS
    }

    // 8. Liberar memoria
    liberar_juego(&juego);
    if(texturaFondo) SDL_DestroyTexture(texturaFondo);
    if(texturaSol) SDL_DestroyTexture(texturaSol);
    if(texturaGirasol) SDL_DestroyTexture(texturaGirasol);
    if(texturaLanza) SDL_DestroyTexture(texturaLanza);
    if(texturaNuez) SDL_DestroyTexture(texturaNuez);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}