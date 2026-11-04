#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ensamblador.h"

void mostrar_ayuda() {
    printf("ens: Entorno ENSMES (Ensamblador Modular Espanol)\n\n");
    printf("Uso:\n");
    printf("  ens compilar <archivo.ensmes>   Compila el codigo fuente a formato JMA\n");
    printf("  ens ejecutar <archivo.jma>      Ejecuta un binario JMA\n");
    printf("  ens version                     Muestra la version\n");
    printf("  ens ayuda                       Muestra este mensaje\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        mostrar_ayuda();
        return 1;
    }

    if (strcmp(argv[1], "ayuda") == 0) {
        mostrar_ayuda();
    } 
    else if (strcmp(argv[1], "version") == 0) {
        printf("ens v1.0 - ENSMES\n");
    } 
    else if (strcmp(argv[1], "compilar") == 0) {
        if (argc < 3) {
            printf("Error: Falta el archivo fuente. Uso: ens compilar <archivo.ensmes>\n");
            return 1;
        }

        FILE *archivo_lectura = fopen(argv[2], "r");
        if (archivo_lectura == NULL) {
            printf("Error: No se pudo abrir el archivo %s\n", argv[2]);
            return 1;
        }

        // Generar nombre .jma automáticamente
        char nombre_salida[256];
        strcpy(nombre_salida, argv[2]);
        char *punto = strrchr(nombre_salida, '.');
        if (punto != NULL) {
            strcpy(punto, ".jma");
        } else {
            strcat(nombre_salida, ".jma");
        }

        FILE *archivo_escritura = fopen(nombre_salida, "wb");
        if (archivo_escritura == NULL) {
            printf("Error: No se pudo crear %s\n", nombre_salida);
            fclose(archivo_lectura);
            return 1;
        }

        printf("Compilando %s -> %s...\n", argv[2], nombre_salida);
        ensamblador_archivo(archivo_lectura, archivo_escritura);
        fclose(archivo_lectura);
        fclose(archivo_escritura);
        printf("Compilacion exitosa.\n");
    } 
    else if (strcmp(argv[1], "ejecutar") == 0) {
        if (argc < 3) {
            printf("Error: Falta el archivo. Uso: ens ejecutar <archivo.jma>\n");
            return 1;
        }
        cargar_y_ejecutar(argv[2]);
    } 
    else {
        printf("Comando no reconocido: %s\n", argv[1]);
        mostrar_ayuda();
        return 1;
    }

    return 0;
}