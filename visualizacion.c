/*gcc visualizacion.c -o ventana -lSDL3 -lSDL3_image*/
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>

#define FILAS 10
#define COLUMNAS 16
#define TAM_CASILLA 62

#define NADA 0
#define GIRASOL 1
#define LANZAGUISANTES 2
#define NUEZ 3

//7x9 es la maatriz de juego para que calce con la imagen

SDL_Texture *cargarTextura(SDL_Renderer *renderizador, char *archivo){
    SDL_Texture *textura = IMG_LoadTexture(renderizador, archivo);
    if(textura == NULL){
        SDL_Log("Error al cargar %s: %s", archivo, SDL_GetError());
    }
    return textura;
}

SDL_FRect posicionSOL = {
    40.0f,
    10.0f,
    60.0f,
    60.0f,
};

SDL_FRect posicionGirasol = {
    120.0f,
    10.0f,
    60.0f,
    60.0f,
};

SDL_FRect posicionLanzaguisantes = {
    200.0f,
    10.0f,
    60.0f,
    60.0f,
};

SDL_FRect posicionNuez = {
    280.0f,
    10.0f,
    60.0f,
    60.0f,
};

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
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };
    
    int plantas[FILAS][COLUMNAS] = {0};

    int plantaArrastrada = NADA;
    float mouseX = 0.f;
    float mouseY = 0.f;

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

    SDL_SetRenderDrawColor(renderizador, 156, 82, 7, 255);
    SDL_Texture *texturaFondo = cargarTextura(renderizador, "mapa_predeterminado.png");
    SDL_Texture *texturaPared = cargarTextura(renderizador, "bloque.png");
    SDL_FRect rectangulo ={ 40.f, 0.f, 500.f, 80.f};
    SDL_Texture *texturaSol = cargarTextura(renderizador, "sol.png");
    SDL_Texture *texturaGirasol = cargarTextura(renderizador, "girasol.png");
    SDL_Texture *texturaguisantes = cargarTextura(renderizador, "lanzaguizantes.png");
    SDL_Texture *texturaNuez = cargarTextura(renderizador, "Nuez.png");
    
    SDL_Texture *texturasPlantas[4] ={NULL, texturaGirasol, texturaguisantes, texturaNuez};

    if(!texturaPared || !texturaFondo || !texturaSol || !texturaGirasol || !texturaguisantes || !texturaNuez){
        SDL_DestroyRenderer(renderizador);
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    while(jugando){

        SDL_Event evento;

        while(SDL_PollEvent(&evento)){
            if(evento.type == SDL_EVENT_QUIT){
                jugando = false;
            }

            else if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT){
                SDL_FPoint punto = {evento.button.x, evento.button.y};
                if(SDL_PointInRectFloat (&punto, &posicionGirasol)){
                    plantaArrastrada = GIRASOL;
                }else if(SDL_PointInRectFloat(&punto, &posicionLanzaguisantes)){
                    plantaArrastrada= LANZAGUISANTES;
                }else if(SDL_PointInRectFloat(&punto, &posicionNuez)){
                    plantaArrastrada = NUEZ;
                }
                mouseX = punto.x;
                mouseY = punto.y;
            }

            else if(evento.type == SDL_EVENT_MOUSE_MOTION){
                mouseX = evento.motion.x;
                mouseY = evento.motion.y;
            }

            else if(evento.type == SDL_EVENT_MOUSE_BUTTON_UP && evento.button.button == SDL_BUTTON_LEFT){
                if(plantaArrastrada != NADA){
                    float x = evento.button.x;
                    float y = evento.button.y;

                    if(x>= 0 && y>=0){
                        int columna = (int)(x/TAM_CASILLA);
                        int fila = (int)(y/TAM_CASILLA);

                        if(fila<FILAS && columna<COLUMNAS && mapa[fila][columna]==0 && plantas[fila][columna]==NADA){
                            plantas[fila][columna] = plantaArrastrada;
                        }
                    }
                    plantaArrastrada = NADA;
                }
            }
        }
    
    SDL_RenderClear(renderizador);

    //For para limitar mi mapa.
    for(int fila=0;fila<FILAS; fila++){
        for(int columna =0; columna<COLUMNAS; columna++){ 
            if(mapa[fila][columna]==1){
                SDL_FRect rectanguloPared = {columna * TAM_CASILLA, fila * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
                SDL_RenderTexture(renderizador, texturaPared, NULL, &rectanguloPared);
            }
        }
    }

    SDL_RenderTexture(renderizador, texturaFondo, NULL,  NULL);

    for(int fila=0; fila<FILAS; fila++){
        for(int columna=0; columna<COLUMNAS; columna++){
            if(plantas[fila][columna]!=NADA){
                SDL_FRect rectanguloplanta = {columna*TAM_CASILLA, fila*TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
                SDL_RenderTexture(renderizador, texturasPlantas[plantas[fila][columna]], NULL, &rectanguloplanta);
            }
        }
    }

    SDL_RenderFillRect(renderizador, &rectangulo);
    SDL_RenderTexture(renderizador, texturaSol, NULL, &posicionSOL);
    SDL_RenderTexture(renderizador, texturaGirasol, NULL, &posicionGirasol);
    SDL_RenderTexture(renderizador, texturaguisantes, NULL, &posicionLanzaguisantes);
    SDL_RenderTexture(renderizador, texturaNuez, NULL, &posicionNuez);

    if(plantaArrastrada!=NADA){
        SDL_FRect rectanguloArrastrado = {mouseX - 30.f, mouseY - 30.f, 60.f, 60.f};
        SDL_RenderTexture(renderizador, texturasPlantas[plantaArrastrada], NULL, &rectanguloArrastrado);
    }
    SDL_RenderPresent(renderizador);

    }

    SDL_DestroyTexture(texturaPared);
    SDL_DestroyTexture(texturaFondo);
    SDL_DestroyTexture(texturaSol);
    SDL_DestroyTexture(texturaGirasol);
    SDL_DestroyTexture(texturaguisantes);
    SDL_DestroyTexture(texturaNuez);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}
