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
#define HERRAMIENTA_PALA 4 // ID para la pala

SDL_Texture *cargarTextura(SDL_Renderer *renderizador, char *archivo){
    SDL_Texture *textura = IMG_LoadTexture(renderizador, archivo);
    if(textura == NULL) SDL_Log("Advertencia: No se encontró %s", archivo);
    return textura;
}

int contar_zombies_vivos(EstadoJuego *juego) {
    int contador = 0;
    for(int i = 0; i < juego->filas; i++){
        for(int j = 0; j < juego->columnas; j++){
            if(juego->clase_entidad[i][j] == ZOMBIE_NORMAL || juego->clase_entidad[i][j] == FLAG_ZOMBIE){
                contador++;
            }
        }
    }
    return contador;
}

int main(){
    srand(time(NULL));
    int filas, columnas;
    
    int **mapa = cargar_mapa("mapa.txt", &filas, &columnas);
    if (mapa == NULL) { printf("Error al cargar mapa.txt\n"); return 1; }
    leer_config();
    
    EstadoJuego juego;
    inicializar_juego(&juego, mapa, filas, columnas);

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

    // TEXTURAS ACTUALIZADAS (nombres coinciden con tu carpeta)
    SDL_Texture *texturaFondo = cargarTextura(renderizador, "mapa_predeterminado.png"); 
    SDL_Texture *texturaSol = cargarTextura(renderizador, "sol.png");         
    SDL_Texture *texturaGirasol = cargarTextura(renderizador, "girasol.png"); 
    SDL_Texture *texturaLanza = cargarTextura(renderizador, "planta.png");    
    SDL_Texture *texturaNuez = cargarTextura(renderizador, "Nuez.png");     
    SDL_Texture *texturaZombie = cargarTextura(renderizador, "zombie.png");     
    SDL_Texture *texturaZombieFlag = cargarTextura(renderizador, "zombie_flag.png");     

    if (texturaGirasol == NULL) texturaGirasol = texturaLanza;
    if (texturaNuez == NULL) texturaNuez = texturaSol; 
    if (texturaZombieFlag == NULL) texturaZombieFlag = texturaZombie;
    
    SDL_FRect barraFondo = {0.f, 0.f, (float)ancho_base, (float)ALTO_BARRA};
    SDL_FRect btnGirasol = {10.f, 10.f, 80.f, 80.f};
    SDL_FRect btnLanza = {100.f, 10.f, 80.f, 80.f};
    SDL_FRect btnNuez = {190.f, 10.f, 80.f, 80.f};
    SDL_FRect rectSolUI = {290.f, 10.f, 80.f, 80.f}; 
    
    // Botón de la Pala
    SDL_FRect btnPala = {390.f, 10.f, 80.f, 80.f};

    int plantaSeleccionada = 0; 
    bool jugando = true;

    while(jugando){
        SDL_Event evento;
        while(SDL_PollEvent(&evento)){
            if(evento.type == SDL_EVENT_QUIT) jugando = false;

            // Guardar partida con la tecla G
            if(evento.type == SDL_EVENT_KEY_DOWN && evento.key.scancode == SDL_SCANCODE_G){
                guardar_partida(&juego, "partida_guardada.txt");
            }

            float mouseX, mouseY;
            SDL_RenderCoordinatesFromWindow(renderizador, evento.button.x, evento.button.y, &mouseX, &mouseY);
            SDL_FPoint pt = {mouseX, mouseY};

            if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT){
                if(SDL_PointInRectFloat(&pt, &btnGirasol)) plantaSeleccionada = GIRASOL;
                else if(SDL_PointInRectFloat(&pt, &btnLanza)) plantaSeleccionada = LANZAGUISANTES;
                else if(SDL_PointInRectFloat(&pt, &btnNuez)) plantaSeleccionada = NUEZ;
                else if(SDL_PointInRectFloat(&pt, &btnPala)) plantaSeleccionada = HERRAMIENTA_PALA;
            }

            if(evento.type == SDL_EVENT_MOUSE_BUTTON_UP && evento.button.button == SDL_BUTTON_LEFT){
                if(plantaSeleccionada != 0 && mouseY > ALTO_BARRA){
                    int columna = (int)((mouseX - offsetX) / anchoCelda);
                    int fila = (int)((mouseY - offsetY) / altoCelda);

                    if(fila >= 0 && fila < filas && columna >= 0 && columna < columnas){
                        if (plantaSeleccionada == HERRAMIENTA_PALA) {
                            eliminar_planta(&juego, fila, columna);
                        } else {
                            colocar_planta(&juego, plantaSeleccionada, fila, columna);
                        }
                    }
                }
            }
        }

        // Solo actualizamos si no hemos ganado/perdido
        // Actualiza logica del juego (soles, plantas, proyectiles, oleadas y zombies)
        if (juego.juego_activo) {
            actualizar_juego(&juego, time(NULL), SDL_GetTicks());
        }

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
                    if(texturaNuez) SDL_RenderTexture(renderizador, texturaNuez, NULL, &rect);
                }
                else if(clase == ZOMBIE_NORMAL){
                    if(texturaZombie) SDL_RenderTexture(renderizador, texturaZombie, NULL, &rect);
                    else {
                        SDL_SetRenderDrawColor(renderizador, 255, 0, 0, 255);
                        SDL_RenderFillRect(renderizador, &rect);
                    }
                }
                else if(clase == FLAG_ZOMBIE){
                    if(texturaZombieFlag) SDL_RenderTexture(renderizador, texturaZombieFlag, NULL, &rect);
                    else {
                        SDL_SetRenderDrawColor(renderizador, 255, 100, 0, 255);
                        SDL_RenderFillRect(renderizador, &rect);
                    }
                }

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
        SDL_RenderFillRect(renderizador, &btnPala);

        if (texturaGirasol) SDL_RenderTexture(renderizador, texturaGirasol, NULL, &btnGirasol);
        if (texturaLanza) SDL_RenderTexture(renderizador, texturaLanza, NULL, &btnLanza);
        if (texturaNuez) SDL_RenderTexture(renderizador, texturaNuez, NULL, &btnNuez);
        if (texturaSol) SDL_RenderTexture(renderizador, texturaSol, NULL, &rectSolUI);
        
        // Texto para la pala
        SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
        SDL_RenderDebugText(renderizador, btnPala.x + 25, btnPala.y + 30, "PALA");

        char texto_soles[50];
        snprintf(texto_soles, sizeof(texto_soles), "Soles: %d", juego.soles);
        SDL_RenderDebugText(renderizador, 500.f, 30.f, texto_soles);

        char texto_vida[50];
        snprintf(texto_vida, sizeof(texto_vida), "Casa: %d", juego.vida_casa);
        SDL_RenderDebugText(renderizador, 500.f, 55.f, texto_vida);

        SDL_RenderDebugText(renderizador, 680.f, 30.f, "Guardar: Presiona 'G'");

        if (juego.vida_casa <= 0) {
            SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 200);
            SDL_RenderFillRect(renderizador, NULL); 
            SDL_SetRenderDrawColor(renderizador, 255, 0, 0, 255);
            SDL_RenderDebugText(renderizador, 350.f, 280.f, "GAME OVER - LOS ZOMBIES GANARON");
        } else if (contar_zombies_vivos(&juego) == 0 && juego.oleada_actual > 0) {
            // Solo gana si ya pasaron algunas oleadas (para no ganar al segundo 1)
            SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 200);
            SDL_RenderFillRect(renderizador, NULL); 
            SDL_SetRenderDrawColor(renderizador, 0, 255, 0, 255);
            SDL_RenderDebugText(renderizador, 350.f, 280.f, "¡VICTORIA! Has defendido la casa.");
        }

        SDL_RenderPresent(renderizador);
        SDL_Delay(16);
    }

    liberar_juego(&juego);
    if(texturaFondo) SDL_DestroyTexture(texturaFondo);
    if(texturaSol) SDL_DestroyTexture(texturaSol);
    if(texturaGirasol) SDL_DestroyTexture(texturaGirasol);
    if(texturaLanza) SDL_DestroyTexture(texturaLanza);
    if(texturaNuez) SDL_DestroyTexture(texturaNuez);
    if(texturaZombie) SDL_DestroyTexture(texturaZombie);
    if(texturaZombieFlag) SDL_DestroyTexture(texturaZombieFlag);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}