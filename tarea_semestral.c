#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "logica.h"
#include "tarea1.h"

#define MENU 0
#define MAPA_PREDETERMINADO 1

#define ALTO_BARRA 100

SDL_Texture *cargarTextura(SDL_Renderer *renderizador, char *archivo){
    SDL_Texture *textura = IMG_LoadTexture(renderizador, archivo);
    if(textura == NULL) SDL_Log("Advertencia: No se encontró %s", archivo);
    return textura;
}
typedef enum { PANTALLA_INICIO, PANTALLA_JUEGO } Pantalla;

int main(){
    srand(time(NULL));
    int filas, columnas;
    
    int **mapa = cargar_mapa("mapa.txt", &filas, &columnas);
    if (mapa == NULL) { printf("Error al cargar mapa.txt\n"); return 1; }
    leer_config();
    
    EstadoJuego juego;
    Pantalla pantalla = PANTALLA_INICIO;
    bool juego_activo = false;

    SDL_FRect btnMapaPred = {320.f, 200.f, 300.f, 100.f};  // en el Inicio
    SDL_FRect btnVolver   = {40.f, 460.f, 100.f, 85.f};   // en la barra del juego

    if(!SDL_Init(SDL_INIT_VIDEO)){ return 1; }

    int offsetX = 195;       
    int offsetY = 115;       
    int anchoCelda = 70;     
    int altoCelda = 75;      

    int ancho_base = 900;
    int alto_base = 600;

    SDL_Window *ventana = NULL;
    SDL_Renderer *renderizador = NULL;

    if(!SDL_CreateWindowAndRenderer("Plants vs Zombies - UdeC", ancho_base, alto_base, SDL_WINDOW_RESIZABLE, &ventana, &renderizador)){
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderizador, ancho_base, alto_base, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_Texture *texturaInicio = cargarTextura(renderizador, "inicio.png");
    SDL_Texture *texturaMPP = cargarTextura(renderizador, "boton_mpp.png");
    SDL_Texture *texturaboton_volver = cargarTextura(renderizador, "boton_volver.jpeg");
    SDL_Texture *texturaFondo = cargarTextura(renderizador, "mapa_predeterminado.png"); 
    SDL_Texture *texturaSol = cargarTextura(renderizador, "sol.png");         
    SDL_Texture *texturaGirasol = cargarTextura(renderizador, "girasol.png"); 
    SDL_Texture *texturaLanza = cargarTextura(renderizador, "planta.png");    
    SDL_Texture *texturaNuez = cargarTextura(renderizador, "Nuez.png");
    SDL_Texture *texturazombie = cargarTextura(renderizador, "zombie.png");     

    if (texturaGirasol == NULL) texturaGirasol = texturaLanza;
    if (texturaNuez == NULL) texturaNuez = texturaSol; 
    
    SDL_FRect barraFondo = {0.f, 0.f, (float)ancho_base, (float)ALTO_BARRA};
    SDL_FRect btnGirasol = {10.f, 10.f, 80.f, 80.f};
    SDL_FRect btnLanza = {100.f, 10.f, 80.f, 80.f};
    SDL_FRect btnNuez = {190.f, 10.f, 80.f, 80.f};
    SDL_FRect rectSolUI = {290.f, 10.f, 80.f, 80.f}; 

    int plantaSeleccionada = 0; 
    bool jugando = true;

    while(jugando){
        SDL_Event evento;
        while(SDL_PollEvent(&evento)){
            if(evento.type == SDL_EVENT_QUIT) jugando = false;
            
            if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evento.type == SDL_EVENT_MOUSE_BUTTON_UP){
                float mouseX, mouseY;
                SDL_RenderCoordinatesFromWindow(renderizador, evento.button.x, evento.button.y, &mouseX, &mouseY);
                SDL_FPoint pt = {mouseX, mouseY};

                if(pantalla==PANTALLA_INICIO){
                    if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT){
                        if(SDL_PointInRectFloat(&pt, &btnMapaPred)){
                            inicializar_juego(&juego, mapa, filas, columnas);
                            juego_activo = true;
                            plantaSeleccionada = 0;
                            pantalla = PANTALLA_JUEGO;
                        }
                    }
                }
                else{
                   if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT){
                        if(SDL_PointInRectFloat(&pt, &btnVolver)){
                            liberar_juego(&juego);
                            juego_activo = false;
                            pantalla = PANTALLA_INICIO; 
                        }
                        else if(SDL_PointInRectFloat(&pt, &btnGirasol)) plantaSeleccionada = GIRASOL;
                        else if(SDL_PointInRectFloat(&pt, &btnLanza)) plantaSeleccionada = LANZAGUISANTES;
                        else if(SDL_PointInRectFloat(&pt, &btnNuez)) plantaSeleccionada = NUEZ;
                    }
                

                if(evento.type == SDL_EVENT_MOUSE_BUTTON_UP && evento.button.button == SDL_BUTTON_LEFT){
                    if(plantaSeleccionada != 0 && mouseY > ALTO_BARRA){
                        int columna = (int)((mouseX - offsetX) / anchoCelda);
                        int fila = (int)((mouseY - offsetY) / altoCelda);

                        if(fila >= 0 && fila < filas && columna >= 0 && columna < columnas){
                            colocar_planta(&juego, plantaSeleccionada, fila, columna);
                        }
                    }
                } 
            }
            }
        }

        if(pantalla == PANTALLA_INICIO){
            SDL_RenderClear(renderizador);

            SDL_RenderDebugText(renderizador, 380.f, 180.f, "Plants vs Zombies - UdeC");
            SDL_RenderTexture(renderizador, texturaInicio, NULL, NULL);
            SDL_RenderTexture(renderizador, texturaMPP, NULL, &btnMapaPred);

            SDL_RenderPresent(renderizador);
            SDL_Delay(16);
            continue;   // no actualiza ni dibuja el juego
        }

        actualizar_juego(&juego, time(NULL));

        SDL_SetRenderDrawColor(renderizador, 30, 30, 30, 255);
        SDL_RenderClear(renderizador);

        if(texturaFondo != NULL){
            SDL_FRect rectFondo = {0.f, (float)ALTO_BARRA, (float)ancho_base, (float)(alto_base - ALTO_BARRA)};
            SDL_RenderTexture(renderizador, texturaFondo, NULL, &rectFondo);
        }

        for(int i = 0; i < juego.filas; i++){
            for(int j = 0; j < juego.columnas; j++){
                SDL_FRect rect = { offsetX + (j * anchoCelda), offsetY + (i * altoCelda), anchoCelda, altoCelda };
                int clase = juego.clase_entidad[i][j];

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
                else if(clase == ZOMBIE_NORMAL || clase == FLAG_ZOMBIE){
                    SDL_RenderTexture(renderizador, texturazombie, NULL, &rect);
                    SDL_RenderRect(renderizador, &rect);
                }
                
                SDL_RenderTexture(renderizador, texturaboton_volver, NULL, &btnVolver);

                if(juego.proyectiles[i][j] == 1){
                    SDL_FRect rectGuisante = { offsetX + (j * anchoCelda) + (anchoCelda/2) - 10, offsetY + (i * altoCelda) + (altoCelda/2) - 10, 20, 20 };
                    SDL_SetRenderDrawColor(renderizador, 50, 255, 50, 255); 
                    SDL_RenderFillRect(renderizador, &rectGuisante);
                }
            }
        }

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

        char texto_soles[50];
        snprintf(texto_soles, sizeof(texto_soles), "Soles: %d", juego.soles);
        SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
        SDL_RenderDebugText(renderizador, 390.f, 30.f, texto_soles);

        char texto_vida[50];
        snprintf(texto_vida, sizeof(texto_vida), "Casa: %d", juego.vida_casa);
        SDL_RenderDebugText(renderizador, 390.f, 55.f, texto_vida);
        
        SDL_RenderDebugText(renderizador, 550.f, 30.f, "Zombies: 1 cada 5s");
        SDL_RenderDebugText(renderizador, 550.f, 55.f, "Velocidad: 3s por paso");

        SDL_RenderPresent(renderizador);
        SDL_Delay(16);
    }

    if(juego_activo) liberar_juego(&juego);
    if(texturaFondo) SDL_DestroyTexture(texturaFondo);
    if(texturaInicio) SDL_DestroyTexture(texturaInicio);
    if(texturaboton_volver) SDL_DestroyTexture(texturaboton_volver);
    if(texturaSol) SDL_DestroyTexture(texturaSol);
    if(texturaGirasol) SDL_DestroyTexture(texturaGirasol);
    if(texturaLanza) SDL_DestroyTexture(texturaLanza);
    if(texturaNuez) SDL_DestroyTexture(texturaNuez);
    if(texturazombie) SDL_DestroyTexture(texturazombie);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}