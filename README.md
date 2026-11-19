# Proyecto Semestral: Plantas vs Zombies Equipo 14

## Compilación

### Versión Gráfica (SDL3)
gcc tarea_semestral.c logica.c tarea1.c -o juego_grafico $(pkg-config --cflags --libs sdl3 sdl3-image)

### Versión Consola
gcc main.c logica.c tarea1.c -o juego_consola

## Ejecución
./juego_grafico

## Controles (Interfaz Gráfica)
- Clic izquierdo en la barra superior: Seleccionar Girasol, Lanzaguisantes, Nuez o Pala.
- Clic izquierdo en el jardín: Plantar o eliminar planta.
- Tecla `G`: Guardar partida en `partida_guardada.txt`.
- Tecla `C`: Cargar partida desde `partida_guardada.txt`.
- Tecla `P`: Activar/desactivar herramienta Pala.
