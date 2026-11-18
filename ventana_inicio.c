/*gcc ventana_inicio.c -o inicio -lSDL3 -lSDL3_image*/
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>

#define largo 960
#define ancho 780

SDL_Texture *cargarTextura(SDL_Renderer *renderizador, char *archivo){
    SDL_Texture *textura = IMG_LoadTexture(renderizador, archivo);
    if(textura == NULL){
        SDL_Log("Error al cargar %s: %s", archivo, SDL_GetError());
    }
    return textura;
}

int estado_juego = 0;
int mostrar_menu(){
    bool jugando = true;

    SDL_Window *ventana = NULL;
    SDL_Renderer *renderizador = NULL;
    SDL_FRect rectangulo ={ 320.f, 200.f, 300.f, 100.f};
    SDL_FRect rectangulo2 ={ 320.f, 400.f, 300.f, 100.f};
    SDL_FPoint pto1 = {320, 200};
    SDL_FPoint pto2 = {320, 400};
    
    if(!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        return 1;
    }

    if(!SDL_CreateWindowAndRenderer("Inicio", largo, ancho, 0, &ventana, &renderizador)){
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        SDL_Quit();
        return 1; 
    }

    SDL_Texture *texturaInicio = cargarTextura(renderizador, "inicio.png");
    SDL_Texture *texturaMPP = cargarTextura(renderizador, "boton_mpp.png");
    SDL_Texture *texturaMPA = cargarTextura(renderizador, "boton_mpa.png");
    
    if(!texturaInicio || !texturaMPP || !texturaMPA){
        SDL_DestroyRenderer(renderizador);
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texturaMapaPredet = cargarTextura(renderizador, "mapa_predeterminado.png");
    SDL_Texture *texturaMapaAleatorio = cargarTextura(renderizador, "Mapa_aleatorio1.png");

    if(!texturaMapaPredet || !texturaMapaAleatorio || !texturaMPA){
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
            if(evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                SDL_FPoint punto_mouse = {evento.button.x, evento.button.y};
                if(estado_juego == 0){
                    if(SDL_PointInRectFloat(&punto_mouse,&rectangulo)){
                        estado_juego = 1;
                    }
                    else if(SDL_PointInRectFloat(&punto_mouse, &rectangulo2)){
                        estado_juego = 2;
                    }
                }
            }
        }
        SDL_RenderClear(renderizador);
        if(estado_juego==0){
            SDL_RenderTexture(renderizador, texturaInicio, NULL,  NULL);
            SDL_RenderTexture(renderizador, texturaMPP, NULL, &rectangulo);
            SDL_RenderTexture(renderizador, texturaMPA, NULL, &rectangulo2);
        }
        else if(estado_juego == 1){
            SDL_RenderTexture(renderizador, texturaMapaPredet,NULL,NULL);
        }
        else if(estado_juego == 2){
            SDL_RenderTexture(renderizador, texturaMapaAleatorio,NULL,NULL);
        }
        SDL_RenderPresent(renderizador);
    }

    SDL_DestroyTexture(texturaInicio);
    SDL_DestroyTexture(texturaMPP);
    SDL_DestroyTexture(texturaMPA);
    SDL_DestroyTexture(texturaMapaPredet);
    SDL_DestroyTexture(texturaMapaAleatorio);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;

}
