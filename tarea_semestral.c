#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "logica.h"
#include "tarea1.h"

#define ALTO_BARRA 100                                    // Altura de la barra superior de HUD y herramientas
#define FPS_OBJETIVO 120                                  // Tasa de refresco objetivo para el bucle gráfico
#define TIEMPO_CUADRO_MS (1000.0 / (double)FPS_OBJETIVO)  // Duracion ideal de cada cuadro en milisegundos
#define ARCHIVO_GUARDADO "partida_guardada.txt"           // Nombre del archivo para guardar y cargar partidas
#define HERRAMIENTA_PALA 99                               // Identificador de la herramienta pala para quitar plantas

// Estados posibles de la interfaz gráfica
typedef enum {
    PANTALLA_INICIO,
    PANTALLA_JUEGO
} Pantalla;

// Carga una textura desde disco y registra una advertencia si falla
static SDL_Texture *cargarTextura(SDL_Renderer *renderizador, const char *archivo) {
    SDL_Texture *textura = IMG_LoadTexture(renderizador, archivo);
    if (textura == NULL) {
        SDL_Log("Advertencia: No se encontró %s (%s)", archivo, SDL_GetError());
    }
    return textura;
}

// Libera la memoria dinamica de una matriz de mapa
static void liberar_mapa(int **mapa, int filas) {
    if (mapa == NULL) {
        return;
    }
    for (int i = 0; i < filas; i++) {
        free(mapa[i]);
    }
    free(mapa);
}

// Pausa lo necesario al final de cada cuadro para respetar el limite de FPS
static void limitar_fps(Uint64 inicio_cuadro_ns) {
    Uint64 fin_cuadro_ns = SDL_GetTicksNS();
    double transcurrido_ms = (double)(fin_cuadro_ns - inicio_cuadro_ns) / 1000000.0;
    if (transcurrido_ms < TIEMPO_CUADRO_MS) {
        double restante_ms = TIEMPO_CUADRO_MS - transcurrido_ms;
        SDL_Delay((Uint32)restante_ms);
    }
}

int main(void) {
    srand((unsigned int)time(NULL)); // Inicializa semilla aleatoria para oleadas y mapas

    // Carga el mapa predeterminado desde archivo y lee los parámetros de configuración
    int filas_pred = 0;
    int columnas_pred = 0;
    int **mapa_pred = cargar_mapa("mapa.txt", &filas_pred, &columnas_pred);
    if (mapa_pred == NULL) {
        printf("Error al cargar mapa.txt\n");
        return 1;
    }

    leer_config();

    // Inicializa la estructura principal del juego en estado inactivo
    EstadoJuego juego;
    juego.juego_activo = 0;
    juego.victoria = 0;
    juego.tipo_entidad = NULL;
    juego.clase_entidad = NULL;
    juego.vida = NULL;
    juego.proyectiles = NULL;
    juego.ultimo_evento = NULL;
    juego.ultimo_proyectil = NULL;
    juego.ultimo_mov_zombie = NULL;

    Pantalla pantalla = PANTALLA_INICIO;
    bool partida_iniciada = false;

    // Botones de la pantalla de inicio y botón para volver al menú
    SDL_FRect btnMapaPred = {300.f, 190.f, 300.f, 75.f};
    SDL_FRect btnMapaRand = {300.f, 285.f, 300.f, 75.f};
    SDL_FRect btnVolver = {25.f, 485.f, 120.f, 75.f};

    // Inicia SDL3 y crea ventana y renderizador con escalado lógico 900x600
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        liberar_mapa(mapa_pred, filas_pred);
        return 1;
    }

    // Coordenadas calibradas sobre el lienzo lógico 900x600 para evitar desfase hacia la derecha
    int offsetX = 195;
    int offsetY = 180;
    int anchoCelda = 60;
    int altoCelda = 75;

    int ancho_base = 900;
    int alto_base = 600;

    SDL_Window *ventana = NULL;
    SDL_Renderer *renderizador = NULL;

    if (!SDL_CreateWindowAndRenderer("Plants vs Zombies - UdeC", ancho_base, alto_base, SDL_WINDOW_RESIZABLE, &ventana, &renderizador)) {
        SDL_Log("Error al crear ventana/renderizador: %s", SDL_GetError());
        liberar_mapa(mapa_pred, filas_pred);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderizador, ancho_base, alto_base, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Carga de recursos gráficos (con respaldo si falta alguna textura)
    SDL_Texture *texturaInicio = cargarTextura(renderizador, "inicio.png");
    SDL_Texture *texturaMPP = cargarTextura(renderizador, "boton_mpp.png");
    SDL_Texture *texturaboton_volver = cargarTextura(renderizador, "boton_volver.jpeg");
    SDL_Texture *texturaFondo = cargarTextura(renderizador, "mapa_predeterminado.png");
    SDL_Texture *texturaSol = cargarTextura(renderizador, "sol.png");
    SDL_Texture *texturaGirasol = cargarTextura(renderizador, "girasol.png");
    SDL_Texture *texturaLanza = cargarTextura(renderizador, "planta.png");
    SDL_Texture *texturaNuez = cargarTextura(renderizador, "Nuez.png");
    SDL_Texture *texturazombie = cargarTextura(renderizador, "zombie.png");
    SDL_Texture *texturaZombieFlag = cargarTextura(renderizador, "zombie_flag.png");

    if (texturaZombieFlag == NULL) {
        texturaZombieFlag = texturazombie;
    }

    if (texturaGirasol == NULL) {
        texturaGirasol = texturaLanza;
    }
    if (texturaNuez == NULL) {
        texturaNuez = texturaSol;
    }

    // Rectángulos de la barra superior y selección de plantas / herramientas
    SDL_FRect barraFondo = {0.f, 0.f, (float)ancho_base, (float)ALTO_BARRA};
    SDL_FRect btnGirasol = {10.f, 10.f, 75.f, 80.f};
    SDL_FRect btnLanza = {95.f, 10.f, 75.f, 80.f};
    SDL_FRect btnNuez = {180.f, 10.f, 75.f, 80.f};
    SDL_FRect btnPala = {265.f, 10.f, 75.f, 80.f};
    SDL_FRect rectSolUI = {350.f, 10.f, 75.f, 80.f};

    int herramientaSeleccionada = 0; // 0 = nada, GIRASOL/LANZAGUISANTES/NUEZ o HERRAMIENTA_PALA
    bool jugando = true;
    char mensaje_estado[96] = "G: Guardar | C: Cargar partida";
    Uint64 tiempo_mensaje_ns = 0;

    while (jugando) {
        Uint64 inicio_cuadro_ns = SDL_GetTicksNS();

        // Procesa eventos de teclado y mouse
        SDL_Event evento;
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_EVENT_QUIT) {
                jugando = false;
            } else if (evento.type == SDL_EVENT_KEY_DOWN && !evento.key.repeat) {
                // Guardar partida con tecla G
                if (evento.key.key == SDLK_G) {
                    if (partida_iniciada && guardar_partida(&juego, ARCHIVO_GUARDADO)) {
                        snprintf(mensaje_estado, sizeof(mensaje_estado), "Partida guardada en %s", ARCHIVO_GUARDADO);
                    } else {
                        snprintf(mensaje_estado, sizeof(mensaje_estado), "No se pudo guardar la partida");
                    }
                    tiempo_mensaje_ns = SDL_GetTicksNS();
                } else if (evento.key.key == SDLK_C) {
                    // Cargar partida guardada con tecla C
                    if (cargar_partida(&juego, ARCHIVO_GUARDADO)) {
                        partida_iniciada = true;
                        herramientaSeleccionada = 0;
                        pantalla = PANTALLA_JUEGO;
                        snprintf(mensaje_estado, sizeof(mensaje_estado), "Partida cargada desde %s", ARCHIVO_GUARDADO);
                    } else {
                        snprintf(mensaje_estado, sizeof(mensaje_estado), "No existe partida guardada");
                    }
                    tiempo_mensaje_ns = SDL_GetTicksNS();
                } else if (evento.key.key == SDLK_P) {
                    // Alternar herramienta pala con tecla P
                    herramientaSeleccionada = (herramientaSeleccionada == HERRAMIENTA_PALA) ? 0 : HERRAMIENTA_PALA;
                }
            } else if (evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evento.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                // Convierte coordenadas de ventana a coordenadas logicas 900x600
                float mouseX = 0.f;
                float mouseY = 0.f;
                SDL_RenderCoordinatesFromWindow(renderizador, evento.button.x, evento.button.y, &mouseX, &mouseY);
                SDL_FPoint pt = {mouseX, mouseY};

                if (pantalla == PANTALLA_INICIO) {
                    if (evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT) {
                        // Iniciar con mapa predeterminado
                        if (SDL_PointInRectFloat(&pt, &btnMapaPred)) {
                            if (partida_iniciada) {
                                liberar_juego(&juego);
                                partida_iniciada = false;
                            }
                            if (inicializar_juego(&juego, mapa_pred, filas_pred, columnas_pred)) {
                                partida_iniciada = true;
                                herramientaSeleccionada = 0;
                                pantalla = PANTALLA_JUEGO;
                            }
                        } else if (SDL_PointInRectFloat(&pt, &btnMapaRand)) {
                            // Generar y cargar un mapa aleatorio nuevo
                            int f_rand = filas_pred > 0 ? filas_pred : 5;
                            int c_rand = columnas_pred > 0 ? columnas_pred : 9;
                            generar_mapa_aleatorio(&f_rand, &c_rand);
                            int filas_carg = 0;
                            int cols_carg = 0;
                            int **mapa_rand = cargar_mapa("mapa_aleatorio.txt", &filas_carg, &cols_carg);
                            if (mapa_rand != NULL) {
                                if (partida_iniciada) {
                                    liberar_juego(&juego);
                                    partida_iniciada = false;
                                }
                                if (inicializar_juego(&juego, mapa_rand, filas_carg, cols_carg)) {
                                    partida_iniciada = true;
                                    herramientaSeleccionada = 0;
                                    pantalla = PANTALLA_JUEGO;
                                }
                                liberar_mapa(mapa_rand, filas_carg);
                            }
                        }
                    }
                } else {
                    // Selección de botones en pantalla de juego
                    if (evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evento.button.button == SDL_BUTTON_LEFT) {
                        if (SDL_PointInRectFloat(&pt, &btnVolver)) {
                            if (partida_iniciada) {
                                liberar_juego(&juego);
                                partida_iniciada = false;
                            }
                            herramientaSeleccionada = 0;
                            pantalla = PANTALLA_INICIO;
                        } else if (SDL_PointInRectFloat(&pt, &btnGirasol)) {
                            herramientaSeleccionada = GIRASOL;
                        } else if (SDL_PointInRectFloat(&pt, &btnLanza)) {
                            herramientaSeleccionada = LANZAGUISANTES;
                        } else if (SDL_PointInRectFloat(&pt, &btnNuez)) {
                            herramientaSeleccionada = NUEZ;
                        } else if (SDL_PointInRectFloat(&pt, &btnPala)) {
                            herramientaSeleccionada = HERRAMIENTA_PALA;
                        }
                    }

                    // Colocar o eliminar planta al soltar el clic sobre una celda válida
                    if (evento.type == SDL_EVENT_MOUSE_BUTTON_UP && evento.button.button == SDL_BUTTON_LEFT) {
                        if (herramientaSeleccionada != 0 && mouseY > ALTO_BARRA && juego.juego_activo) {
                            int columna = (int)((mouseX - (float)offsetX) / (float)anchoCelda);
                            int fila = (int)((mouseY - (float)offsetY) / (float)altoCelda);

                            if (fila >= 0 && fila < juego.filas && columna >= 0 && columna < juego.columnas) {
                                if (herramientaSeleccionada == HERRAMIENTA_PALA) {
                                    eliminar_planta(&juego, fila, columna);
                                } else {
                                    colocar_planta(&juego, herramientaSeleccionada, fila, columna);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Restaura el mensaje de ayuda tras 3 segundos
        if (tiempo_mensaje_ns != 0 && (SDL_GetTicksNS() - tiempo_mensaje_ns) > 3000000000ULL) {
            snprintf(mensaje_estado, sizeof(mensaje_estado), "G: Guardar | C: Cargar partida");
            tiempo_mensaje_ns = 0;
        }

        // Dibujo de la pantalla de inicio
        if (pantalla == PANTALLA_INICIO) {
            SDL_SetRenderDrawColor(renderizador, 20, 20, 20, 255);
            SDL_RenderClear(renderizador);

            if (texturaInicio) {
                SDL_RenderTexture(renderizador, texturaInicio, NULL, NULL);
            }

            if (texturaMPP) {
                SDL_RenderTexture(renderizador, texturaMPP, NULL, &btnMapaPred);
            } else {
                SDL_SetRenderDrawColor(renderizador, 60, 140, 60, 255);
                SDL_RenderFillRect(renderizador, &btnMapaPred);
            }

            SDL_SetRenderDrawColor(renderizador, 50, 95, 160, 255);
            SDL_RenderFillRect(renderizador, &btnMapaRand);
            SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
            SDL_RenderRect(renderizador, &btnMapaRand);
            SDL_RenderDebugText(renderizador, btnMapaRand.x + 75.f, btnMapaRand.y + 32.f, "MAPA ALEATORIO");
            SDL_RenderDebugText(renderizador, 360.f, 155.f, "Plants vs Zombies - UdeC");
            SDL_RenderDebugText(renderizador, 320.f, 380.f, mensaje_estado);

            SDL_RenderPresent(renderizador);
            limitar_fps(inicio_cuadro_ns);
            continue;
        }

        // Avanza la simulación del juego usando tiempo en segundos y milisegundos
        actualizar_juego(&juego, time(NULL), SDL_GetTicks());

        SDL_SetRenderDrawColor(renderizador, 30, 30, 30, 255);
        SDL_RenderClear(renderizador);

        // Fondo del jardín debajo de la barra superior
        if (texturaFondo != NULL) {
            SDL_FRect rectFondo = {0.f, (float)ALTO_BARRA, (float)ancho_base, (float)(alto_base - ALTO_BARRA)};
            SDL_RenderTexture(renderizador, texturaFondo, NULL, &rectFondo);
        }

        // Dibuja plantas, zombies y proyectiles en cada celda (i, j)
        for (int i = 0; i < juego.filas; i++) {
            for (int j = 0; j < juego.columnas; j++) {
                SDL_FRect rect = {
                    (float)offsetX + ((float)j * (float)anchoCelda),
                    (float)offsetY + ((float)i * (float)altoCelda),
                    (float)anchoCelda,
                    (float)altoCelda
                };
                int clase = juego.clase_entidad[i][j];

                if (clase == GIRASOL) {
                    if (texturaGirasol) {
                        SDL_RenderTexture(renderizador, texturaGirasol, NULL, &rect);
                    }
                } else if (clase == LANZAGUISANTES) {
                    if (texturaLanza) {
                        SDL_RenderTexture(renderizador, texturaLanza, NULL, &rect);
                    }
                } else if (clase == NUEZ) {
                    if (texturaNuez) {
                        SDL_RenderTexture(renderizador, texturaNuez, NULL, &rect);
                    }
                } else if (clase == ZOMBIE_NORMAL) {
                    if (texturazombie) {
                        SDL_RenderTexture(renderizador, texturazombie, NULL, &rect);
                    } else {
                        SDL_SetRenderDrawColor(renderizador, 220, 40, 40, 255);
                        SDL_RenderFillRect(renderizador, &rect);
                    }
                } else if (clase == FLAG_ZOMBIE) {
                    if (texturaZombieFlag) {
                        SDL_RenderTexture(renderizador, texturaZombieFlag, NULL, &rect);
                    }
                }

                // Proyectil dibujado encima para que atraviese plantas amigas sin ocultarse
                if (juego.proyectiles[i][j] == 1) {
                    SDL_FRect rectGuisante = {
                        (float)offsetX + ((float)j * (float)anchoCelda) + ((float)anchoCelda / 2.f) - 8.f,
                        (float)offsetY + ((float)i * (float)altoCelda) + ((float)altoCelda / 2.f) - 8.f,
                        16.f,
                        16.f
                    };
                    SDL_SetRenderDrawColor(renderizador, 50, 255, 50, 255);
                    SDL_RenderFillRect(renderizador, &rectGuisante);
                }
            }
        }

        // Botón volver a inicio
        if (texturaboton_volver) {
            SDL_RenderTexture(renderizador, texturaboton_volver, NULL, &btnVolver);
        } else {
            SDL_SetRenderDrawColor(renderizador, 180, 45, 45, 255);
            SDL_RenderFillRect(renderizador, &btnVolver);
            SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
            SDL_RenderDebugText(renderizador, btnVolver.x + 20.f, btnVolver.y + 32.f, "INICIO");
        }

        // Barra superior con herramientas y marcador de estado
        SDL_SetRenderDrawColor(renderizador, 80, 80, 80, 255);
        SDL_RenderFillRect(renderizador, &barraFondo);

        SDL_SetRenderDrawColor(renderizador, 120, 120, 120, 255);
        SDL_RenderFillRect(renderizador, &btnGirasol);
        SDL_RenderFillRect(renderizador, &btnLanza);
        SDL_RenderFillRect(renderizador, &btnNuez);
        SDL_RenderFillRect(renderizador, &btnPala);

        if (texturaGirasol) {
            SDL_RenderTexture(renderizador, texturaGirasol, NULL, &btnGirasol);
        }
        if (texturaLanza) {
            SDL_RenderTexture(renderizador, texturaLanza, NULL, &btnLanza);
        }
        if (texturaNuez) {
            SDL_RenderTexture(renderizador, texturaNuez, NULL, &btnNuez);
        }
        if (texturaSol) {
            SDL_RenderTexture(renderizador, texturaSol, NULL, &rectSolUI);
        }

        SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
        SDL_RenderDebugText(renderizador, btnPala.x + 20.f, btnPala.y + 35.f, "PALA");

        char texto_soles[64];
        snprintf(texto_soles, sizeof(texto_soles), "Soles: %d", juego.soles);
        SDL_RenderDebugText(renderizador, 440.f, 20.f, texto_soles);

        char texto_vida[64];
        snprintf(texto_vida, sizeof(texto_vida), "Casa: %d", juego.vida_casa);
        SDL_RenderDebugText(renderizador, 440.f, 42.f, texto_vida);

        char texto_oleada[64];
        if (juego.en_descanso) {
            snprintf(texto_oleada, sizeof(texto_oleada), "Oleada: %d/%d (Descanso)", juego.oleada_actual, juego.max_oleadas);
        } else {
            snprintf(texto_oleada, sizeof(texto_oleada), "Oleada: %d/%d", juego.oleada_actual, juego.max_oleadas);
        }
        SDL_RenderDebugText(renderizador, 440.f, 64.f, texto_oleada);

        SDL_RenderDebugText(renderizador, 600.f, 20.f, "Zombies: 1 cada 5s | 120 FPS");
        SDL_RenderDebugText(renderizador, 600.f, 42.f, "P/Clic: Pala | Velocidad: 3s");
        SDL_RenderDebugText(renderizador, 600.f, 64.f, mensaje_estado);

        // Cartel final de victoria o derrota cuando termina la partida
        if (juego.juego_activo == 0) {
            SDL_FRect bannerFin = {230.f, 250.f, 440.f, 90.f};
            SDL_SetRenderDrawColor(renderizador, 20, 20, 20, 230);
            SDL_RenderFillRect(renderizador, &bannerFin);
            if (juego.victoria) {
                SDL_SetRenderDrawColor(renderizador, 80, 255, 120, 255);
                SDL_RenderRect(renderizador, &bannerFin);
                SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
                SDL_RenderDebugText(renderizador, 310.f, 285.f, "Victoria! Sobreviviste todas las oleadas");
            } else {
                SDL_SetRenderDrawColor(renderizador, 255, 80, 80, 255);
                SDL_RenderRect(renderizador, &bannerFin);
                SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
                SDL_RenderDebugText(renderizador, 320.f, 285.f, "Partida terminada - Casa destruida");
            }
        }

        SDL_RenderPresent(renderizador);
        limitar_fps(inicio_cuadro_ns);
    }

    // Limpieza ordenada de memoria y recursos SDL3
    if (partida_iniciada) {
        liberar_juego(&juego);
    }
    liberar_mapa(mapa_pred, filas_pred);

    if (texturaFondo) SDL_DestroyTexture(texturaFondo);
    if (texturaInicio) SDL_DestroyTexture(texturaInicio);
    if (texturaMPP && texturaMPP != texturaInicio) SDL_DestroyTexture(texturaMPP);
    if (texturaboton_volver) SDL_DestroyTexture(texturaboton_volver);
    if (texturaSol) SDL_DestroyTexture(texturaSol);
    if (texturaGirasol && texturaGirasol != texturaLanza) SDL_DestroyTexture(texturaGirasol);
    if (texturaLanza) SDL_DestroyTexture(texturaLanza);
    if (texturaNuez && texturaNuez != texturaSol) SDL_DestroyTexture(texturaNuez);
    if (texturazombie) SDL_DestroyTexture(texturazombie);

    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}
