/*gcc visualizacion.c -o ventana -lSDL3 -lSDL3_image*/
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>

#define FILAS 10
#define COLUMNAS 16
#define TAM_CASILLA 62

//7x9 es la maatriz de juego para que calce con la imagen

SDL_Texture *cargarTextura(SDL_Renderer *renderizador, char *archivo){
    SDL_Texture *textura = IMG_LoadTexture(renderizador, archivo);
    if(textura == NULL){
        SDL_Log("Error al cargar %s: %s", archivo, SDL_GetError());
    }
    return textura;
}

int main(){
    int mapa[FILAS][COLUMNAS] ={
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};

    int filaJugador = 2;
    int columnaJugador = 4;
    bool jugando = true;

    SDL_Window *ventana = NULL;
    SDL_Renderer *renderizador = NULL;

        
    if(!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        return 1;
    }
        
    if(!SDL_CreateWindowAndRenderer("Mapa de juego", COLUMNAS*TAM_CASILLA, FILAS*TAM_CASILLA, 0, &ventana, &renderizador)){
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        SDL_Quit();
        return 1; 
    }
    SDL_Texture *texturaFondo = cargarTextura(renderizador, "Mapa_aleatorio1.png");
    SDL_Texture *texturaPared = cargarTextura(renderizador, "bloque.png");
    SDL_Texture *texturaJugador = cargarTextura(renderizador, "planta.png");
    

    if(!texturaPared || !texturaJugador){
        SDL_DestroyRenderer(renderizador);
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    while(jugando){

        SDL_Event evento;

        while(SDL_PollEvent(&evento)){
            if(evento.type == SDL_EVENT_QUIT)
                jugando = false;
            if(evento.type== SDL_EVENT_KEY_DOWN && !evento.key.repeat){

                int siguienteFila = filaJugador;
                int siguienteColumna  = columnaJugador;

                switch (evento.key.scancode){
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    siguienteFila--;
                    break; 

                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    siguienteFila++;
                    break; 

                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    siguienteColumna--;
                    break;

                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    siguienteColumna++;
                    break;
                        
                case SDL_SCANCODE_ESCAPE:
                    jugando = false;
                    break;

                default:
                    break;
                }  
                if(siguienteFila>=0 && siguienteFila<FILAS && siguienteColumna>=0 && siguienteColumna<COLUMNAS && mapa[siguienteFila][siguienteColumna]==0){
                    filaJugador = siguienteFila;
                    columnaJugador = siguienteColumna;
                }
            }
        }
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);

    for(int fila=0;fila<FILAS; fila++){
        for(int columna =0; columna<COLUMNAS; columna++){ 
            if(mapa[fila][columna]==1){
                SDL_FRect rectanguloPared = {columna * TAM_CASILLA, fila * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
                SDL_RenderTexture(renderizador, texturaPared, NULL, &rectanguloPared);
            }
        }
    }
    SDL_FRect rectangulojugador = {columnaJugador * TAM_CASILLA, filaJugador * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
    SDL_RenderTexture(renderizador, texturaFondo, NULL,  NULL);
    SDL_RenderTexture(renderizador, texturaJugador, NULL, &rectangulojugador);

    SDL_RenderPresent(renderizador);
    SDL_Delay(16);
}


SDL_DestroyTexture(texturaJugador);
SDL_DestroyTexture(texturaPared);
SDL_DestroyTexture(texturaFondo);
SDL_DestroyRenderer(renderizador);
SDL_DestroyWindow(ventana);
SDL_Quit();

return 0;
}
