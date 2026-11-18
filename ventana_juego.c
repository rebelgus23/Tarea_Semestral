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

    // --- CALIBRACIÓN DE LA CUADRÍCULA ---
    // Cambia estos valores si los clics no coinciden con el pasto
    int offsetX = 190;       // Margen izquierdo (donde empieza el pasto)
    int offsetY = 60;        // Margen superior (donde empieza el pasto)
    int anchoCelda = 70;     // Ancho de cada cuadro
    int altoCelda = 80;      // Alto de cada cuadro

    // Calculamos el tamaño de la ventana en base a la imagen (aprox 900x600)
    int ancho_base = 900;
    int alto_base = 600;

    SDL_Window *ventana = NULL;
    SDL_Renderer *renderizador = NULL;

    if(!SDL_CreateWindowAndRenderer("Plants vs Zombies - UdeC", ancho_base, alto_base, SDL_WINDOW_RESIZABLE, &ventana, &renderizador)){
        SDL_Log("Error al crear ventana: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderizador, ancho_base, alto_base, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // 3. Cargar texturas
    SDL_Texture *texturaFondo = cargarTextura(renderizador, "mapa_predeterminado.png"); 
    SDL_Texture *texturaSol = cargarTextura(renderizador, "sol.jpg");         
    SDL_Texture *texturaGirasol = cargarTextura(renderizador, "girasol.jpg"); 
    SDL_Texture *texturaLanza = cargarTextura(renderizador, "planta.png");    
    SDL_Texture *texturaNuez = cargarTextura(renderizador, "canela.jpg");     

    if (texturaGirasol == NULL) texturaGirasol = texturaLanza;
    if (texturaNuez == NULL) texturaNuez = texturaSol; 
    
    // Barra superior
    SDL_FRect barraFondo = {0.f, 0.f, (float)ancho_base, (float)ALTO_BARRA};
    
    // Botones UI
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

            // mapa
            // Plantar calculando con los offsets
            if(evento.type == SDL_EVENT_MOUSE_BUTTON_UP && evento.button.button == SDL_BUTTON_LEFT){
                if(plantaSeleccionada != 0 && mouseY > ALTO_BARRA){
                    
                    int columna = (int)((mouseX - offsetX) / anchoCelda);
                    int fila = (int)((mouseY - offsetY) / altoCelda);

                    // Revisar que el clic esté dentro de los límites del mapa
                    if(fila >= 0 && fila < filas && columna >= 0 && columna < columnas){
                        int exito = colocar_planta(&juego, plantaSeleccionada, fila, columna);
                        if(exito) printf("Planta colocada en %d, %d\n", fila, columna);
                        else printf("No se pudo plantar (falta plata o casilla ocupada).\n");
                    }
                }
            }
        }

        // 5. Actualizar lógica
        actualizar_juego(&juego, time(NULL));

        // 6. Dibujar
        SDL_SetRenderDrawColor(renderizador, 30, 30, 30, 255);
        SDL_RenderClear(renderizador);

        // Fondo (Ocupa toda la ventana menos la barra)
        if(texturaFondo != NULL){
            SDL_FRect rectFondo = {0.f, (float)ALTO_BARRA, (float)ancho_base, (float)(alto_base - ALTO_BARRA)};
            SDL_RenderTexture(renderizador, texturaFondo, NULL, &rectFondo);
        }

        // Entidades
        for(int i = 0; i < juego.filas; i++){
            for(int j = 0; j < juego.columnas; j++){
                // Aplicamos offsets al dibujar
                SDL_FRect rect = { offsetX + (j * anchoCelda), offsetY + (i * altoCelda), anchoCelda, altoCelda };
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
                        SDL_SetRenderDrawColor(renderizador, 139, 69, 19, 255); 
                        SDL_RenderRect(renderizador, &rect);
                    }
                }
                // Dibujar Zombies
                else if(clase == ZOMBIE_NORMAL || clase == FLAG_ZOMBIE){
                    SDL_SetRenderDrawColor(renderizador, 200, 0, 0, 255);
                    SDL_RenderFillRect(renderizador, &rect);
                    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
                    SDL_RenderRect(renderizador, &rect);
                }

                // Dibujar Proyectiles (Guisantes)
                if(juego.proyectiles[i][j] == 1){
                    SDL_FRect rectGuisante = { offsetX + (j * anchoCelda) + (anchoCelda/2) - 15, offsetY + (i * altoCelda) + (altoCelda/2) - 15, 30, 30 };
                    SDL_SetRenderDrawColor(renderizador, 255, 255, 0, 255);
                    SDL_RenderFillRect(renderizador, &rectGuisante);
                    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255); 
                    SDL_RenderRect(renderizador, &rectGuisante);
                }
            }
        }

        // 7. Dibujar UI
        SDL_SetRenderDrawColor(renderizador, 80, 80, 80, 255);
        SDL_RenderFillRect(renderizador, &barraFondo);

        SDL_SetRenderDrawColor(renderizador, 120, 120, 120, 255);
        SDL_RenderFillRect(renderizador, &btnGirasol);
        SDL_RenderFillRect(renderizador, &btnLanza);
        SDL_RenderFillRect(renderizador, &btnNuez);

        if (texturaGirasol) SDL_RenderTexture(renderizador, texturaGirasol, NULL, &btnGirasol);
        if (texturaLanza) SDL_RenderTexture(renderizador, texturaLanza, NULL, &btnLanza);
        if (texturaNuez) SDL_RenderTexture(renderizador, texturaNuez, NULL, &btnNuez);

        if (texturaSol) SDL_RenderTexture(renderizador, texturaSol, NULL, &rectSolUI);

        SDL_SetRenderDrawColor(renderizador, 255, 215, 0, 255);
        SDL_FRect rectSoles = {380.f, 30.f, (float)(juego.soles * 2), 40.f}; 
        SDL_RenderFillRect(renderizador, &rectSoles);

        SDL_RenderPresent(renderizador);
        SDL_Delay(16);
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