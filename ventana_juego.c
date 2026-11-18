/*gcc ventana_juego.c -o vjuego -lSDL3 -lSDL3_image*/
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>

/* Tamano de la ventana del menu de inicio */
#define ANCHO_MENU 960
#define ALTO_MENU 780

/* Tamano del mapa de juego */
#define FILAS 10
#define COLUMNAS 16
#define TAM_CASILLA 62

/* Tipos de planta */
#define NADA 0
#define GIRASOL 1
#define LANZAGUISANTES 2
#define NUEZ 3

/* Estados del programa */
#define MENU 0
#define MAPA_PREDETERMINADO 1
#define MAPA_ALEATORIO 2

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

    /* Estado general del programa: menu o alguno de los dos mapas */
    int estado_juego = MENU;

    /* Estado del arrastre */
    int plantaArrastrada = NADA;
    float mouseX = 0.f;
    float mouseY = 0.f;

    bool jugando = true;

    SDL_Window *ventana = NULL;
    SDL_Renderer *renderizador = NULL;

    /* Botones del menu */
    SDL_FRect botonPredeterminado = {320.f, 200.f, 300.f, 100.f};
    SDL_FRect botonAleatorio = {320.f, 400.f, 300.f, 100.f};

    /* Barra superior donde estan las plantas */
    SDL_FRect barraPlantas = {40.f, 0.f, 500.f, 80.f};

    if(!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        return 1;
    }

    /* La ventana parte con el tamano del menu */
    if(!SDL_CreateWindowAndRenderer("Juego", ANCHO_MENU, ALTO_MENU, 0, &ventana, &renderizador)){
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderizador, 156, 82, 7, 255);

    /* Texturas del menu */
    SDL_Texture *texturaInicio = cargarTextura(renderizador, "inicio.png");
    SDL_Texture *texturaMPP = cargarTextura(renderizador, "boton_mpp.png");
    SDL_Texture *texturaMPA = cargarTextura(renderizador, "boton_mpa.png");

    /* Texturas del juego */
    SDL_Texture *texturaMapaPredet = cargarTextura(renderizador, "mapa_predeterminado.png");
    SDL_Texture *texturaMapaAleatorio = cargarTextura(renderizador, "Mapa_aleatorio1.png");
    SDL_Texture *texturaPared = cargarTextura(renderizador, "bloque.png");
    SDL_Texture *texturaSol = cargarTextura(renderizador, "sol.png");
    SDL_Texture *texturaGirasol = cargarTextura(renderizador, "girasol.png");
    SDL_Texture *texturaguisantes = cargarTextura(renderizador, "lanzaguizantes.png");
    SDL_Texture *texturaNuez = cargarTextura(renderizador, "Nuez.png");

    SDL_Texture *texturasPlantas[4] = {NULL, texturaGirasol, texturaguisantes, texturaNuez};

    if(!texturaInicio || !texturaMPP || !texturaMPA ||
       !texturaMapaPredet || !texturaMapaAleatorio || !texturaPared ||
       !texturaSol || !texturaGirasol || !texturaguisantes || !texturaNuez){
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

            /* ---------- EVENTOS DEL MENU ---------- */
            else if(estado_juego == MENU){
                if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                    SDL_FPoint punto_mouse = {evento.button.x, evento.button.y};

                    if(SDL_PointInRectFloat(&punto_mouse, &botonPredeterminado)){
                        estado_juego = MAPA_PREDETERMINADO;
                    }
                    else if(SDL_PointInRectFloat(&punto_mouse, &botonAleatorio)){
                        estado_juego = MAPA_ALEATORIO;
                    }

                    /* Si se eligio un mapa, la ventana pasa al tamano del juego */
                    if(estado_juego != MENU){
                        SDL_SetWindowSize(ventana, COLUMNAS*TAM_CASILLA, FILAS*TAM_CASILLA);
                        SDL_SetWindowPosition(ventana, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                    }
                }
            }

            /* ---------- EVENTOS DEL JUEGO (arrastrar plantas) ---------- */
            else{
                if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT){
                    SDL_FPoint punto = {evento.button.x, evento.button.y};
                    if(SDL_PointInRectFloat(&punto, &posicionGirasol)){
                        plantaArrastrada = GIRASOL;
                    }else if(SDL_PointInRectFloat(&punto, &posicionLanzaguisantes)){
                        plantaArrastrada = LANZAGUISANTES;
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

                        if(x >= 0 && y >= 0){
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
        }

        SDL_RenderClear(renderizador);

        /* ---------- DIBUJAR EL MENU ---------- */
        if(estado_juego == MENU){
            SDL_RenderTexture(renderizador, texturaInicio, NULL, NULL);
            SDL_RenderTexture(renderizador, texturaMPP, NULL, &botonPredeterminado);
            SDL_RenderTexture(renderizador, texturaMPA, NULL, &botonAleatorio);
        }

        /* ---------- DIBUJAR EL JUEGO ---------- */
        else{
            //For para limitar mi mapa.
            for(int fila=0; fila<FILAS; fila++){
                for(int columna=0; columna<COLUMNAS; columna++){
                    if(mapa[fila][columna]==1){
                        SDL_FRect rectanguloPared = {columna * TAM_CASILLA, fila * TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
                        SDL_RenderTexture(renderizador, texturaPared, NULL, &rectanguloPared);
                    }
                }
            }

            /* El fondo depende del mapa elegido en el menu */
            if(estado_juego == MAPA_PREDETERMINADO){
                SDL_RenderTexture(renderizador, texturaMapaPredet, NULL, NULL);
            }else{
                SDL_RenderTexture(renderizador, texturaMapaAleatorio, NULL, NULL);
            }

            for(int fila=0; fila<FILAS; fila++){
                for(int columna=0; columna<COLUMNAS; columna++){
                    if(plantas[fila][columna]!=NADA){
                        SDL_FRect rectanguloplanta = {columna*TAM_CASILLA, fila*TAM_CASILLA, TAM_CASILLA, TAM_CASILLA};
                        SDL_RenderTexture(renderizador, texturasPlantas[plantas[fila][columna]], NULL, &rectanguloplanta);
                    }
                }
            }

            SDL_RenderFillRect(renderizador, &barraPlantas);
            SDL_RenderTexture(renderizador, texturaSol, NULL, &posicionSOL);
            SDL_RenderTexture(renderizador, texturaGirasol, NULL, &posicionGirasol);
            SDL_RenderTexture(renderizador, texturaguisantes, NULL, &posicionLanzaguisantes);
            SDL_RenderTexture(renderizador, texturaNuez, NULL, &posicionNuez);

            if(plantaArrastrada!=NADA){
                SDL_FRect rectanguloArrastrado = {mouseX - 30.f, mouseY - 30.f, 60.f, 60.f};
                SDL_RenderTexture(renderizador, texturasPlantas[plantaArrastrada], NULL, &rectanguloArrastrado);
            }
        }

        SDL_RenderPresent(renderizador);
    }

    SDL_DestroyTexture(texturaInicio);
    SDL_DestroyTexture(texturaMPP);
    SDL_DestroyTexture(texturaMPA);
    SDL_DestroyTexture(texturaMapaPredet);
    SDL_DestroyTexture(texturaMapaAleatorio);
    SDL_DestroyTexture(texturaPared);
    SDL_DestroyTexture(texturaSol);
    SDL_DestroyTexture(texturaGirasol);
    SDL_DestroyTexture(texturaguisantes);
    SDL_DestroyTexture(texturaNuez);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}