#ifndef ENSAMBLADOR_H
#define ENSAMBLADOR_H
#include <stdio.h> 
// En ensamblador.h
void cargar_y_ejecutar(const char *nombre_archivo);
void ensamblador_archivo(FILE *archivo_lectura, FILE *archivo_estructura);

#endif