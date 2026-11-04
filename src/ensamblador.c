// ENSAMBLADOR MODULAR ESPAÑOL x86_64
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>
#include "ensamblador.h"
#define MAX_ETIQUETAS 100
#define MAX_SALTOS_PENDIENTES 100
#define TAMANO_CABECERA 80 // 4+4+8+8+8+8+8+32 = 80 bytes

typedef struct {
    char nombre_etiqueta[50];
    long posicion_hueco; // Dónde están los 4 bytes a rellenar
} SaltoPendiente;

SaltoPendiente saltos_pendientes[MAX_SALTOS_PENDIENTES];
int num_saltos_pendientes = 0;

typedef struct {
    char nombre[50];
    long posicion;
} Etiqueta;

Etiqueta tabla_etiquetas[MAX_ETIQUETAS];
int num_etiquetas = 0;

void imprimir_numero(int num, FILE *archivo_escritura)
{
    char buffer[70] ;
    int i ;
}

void ensamblador_archivo(FILE *archivo_lectura, FILE *archivo_escritura) 
{
    int es_64 ;
    int en_seccion_datos ;
    char linea[1500] ;      // Buffer para leer cada línea
    char *token ;           // Declarar puntero para instrucción sin operando
    char *token_registros ; // Declarar puntero para los registros
    char *token_numero ;    // Declarar puntero para los inmediatos
    
    long offset_codigo = -1;
    long offset_datos = -1;

    // Escribir el hueco de 80 bytes para la cabecera
    for (int i = 0; i < TAMANO_CABECERA; i++) {
        fputc(0x00, archivo_escritura);
    }

    while(fgets(linea, 1500, archivo_lectura) != NULL) {
        es_64 = 0; // <--- VITAL: Reiniciar la bandera en cada línea
        
        // 1. Tokenización de la línea
        // Cortamos el texto en espacios, comas y saltos de linea
        token = strtok(linea, " ,\n#") ;
        token_numero = strtok(NULL, " ,\n") ;
        token_registros = strtok(NULL, " ,\n") ;
        
        if (token == NULL) {
            continue; // Si la línea está vacía, saltamos a la siguiente
        }

        // Si el token termina en ':', es una etiqueta
        if (token != NULL && token[strlen(token) - 1] == ':') {
            // Quitamos los dos puntos del final
            token[strlen(token) - 1] = '\0';
            
            // 1. Guardamos la etiqueta y su posición (como ya hacía)
            if (num_etiquetas < MAX_ETIQUETAS) {
                strcpy(tabla_etiquetas[num_etiquetas].nombre, token);
                tabla_etiquetas[num_etiquetas].posicion = ftell(archivo_escritura);
                num_etiquetas++;
            }
            
            // 2. RESOLVER SALTOS PENDIENTES
            // Si habíamos intentado saltar a esta etiqueta antes de conocerla,
            // ahora que sabemos dónde está, rellenamos el hueco.
            for (int i = 0; i < num_saltos_pendientes; i++) {
                if (saltos_pendientes[i].posicion_hueco != -1 && 
                    strcmp(saltos_pendientes[i].nombre_etiqueta, token) == 0) {
                    
                    // Calculamos la distancia
                    long pos_instruccion_final = saltos_pendientes[i].posicion_hueco + 4;
                    int distancia = (int)(ftell(archivo_escritura) - pos_instruccion_final);
                    
                    // Nos movemos al hueco del archivo binario
                    fseek(archivo_escritura, saltos_pendientes[i].posicion_hueco, SEEK_SET); 
                    // Escribimos la distancia real
                    fwrite(&distancia, sizeof(int), 1, archivo_escritura);
                    // Volvemos al final del archivo para seguir ensamblando
                    fseek(archivo_escritura, 0, SEEK_END);
                    
                    // Marcamos este salto como resuelto para que no se vuelva a procesar
                    saltos_pendientes[i].posicion_hueco = -1; 
                }
            }
            
            continue; // Pasamos a la siguiente línea
        }

        // 3. GESTIÓN DE SECCIONES Y DATOS
        if (strcmp(token, ".datos") == 0) {
            en_seccion_datos = 1;
            if(offset_datos == -1){
                offset_datos = ftell(archivo_escritura);
            }
            continue;
        }
        else if (strcmp(token, ".codigo") == 0) {
            en_seccion_datos = 0;
            if(offset_codigo == -1){
                offset_codigo = ftell(archivo_escritura);
            }
            continue;
        }
        // DIRECTIVAS DE DATOS
        else if (strcmp(token, "definir_byte") == 0) {
            int valor = atoi(token_numero);
            fputc(valor, archivo_escritura);
            continue;
        }
        else if (strcmp(token, "definir_palabra") == 0) {
            short valor = (short)atoi(token_numero);
            fwrite(&valor, sizeof(short), 1, archivo_escritura);
            continue;
        }
        else if (strcmp(token, "definir_doble") == 0) {
            int valor = atoi(token_numero);
            fwrite(&valor, sizeof(int), 1, archivo_escritura);
            continue;
        }
        else if (strcmp(token, "definir_cuadruple") == 0) {
            long long valor = atoll(token_numero);
            fwrite(&valor, sizeof(long long), 1, archivo_escritura);
            continue;
        }
        else if (strcmp(token, "reservar_byte") == 0) {
            int cantidad = atoi(token_numero);
            for (int i = 0; i < cantidad; i++) { fputc(0x00, archivo_escritura); }
            continue;
        }
        else if (strcmp(token, "reservar_palabra") == 0) {
            int cantidad = atoi(token_numero);
            for (int i = 0; i < cantidad * 2; i++) { fputc(0x00, archivo_escritura); }
            continue;
        }
        else if (strcmp(token, "reservar_doble") == 0) {
            int cantidad = atoi(token_numero);
            for (int i = 0; i < cantidad * 4; i++) { fputc(0x00, archivo_escritura); }
            continue;
        }
        else if (strcmp(token, "reservar_cuadruple") == 0) {
            int cantidad = atoi(token_numero);
            for (int i = 0; i < cantidad * 8; i++) { fputc(0x00, archivo_escritura); }
            continue;
        }

        // 4. INSTRUCCIONES
        if(strcmp(token, "pausa") == 0) {
            fputc(0x90, archivo_escritura) ;
        } 
        else if(strcmp(token, "volver") == 0) {
            fputc(0xC3, archivo_escritura) ;
        }
        else if(strcmp(token, "empujar") == 0) {
            if(strcmp(token_numero, "r_acumulador64") == 0) { fputc(0x50, archivo_escritura) ; }
            else if(strcmp(token_numero, "r_base64") == 0) { fputc(0x53, archivo_escritura) ; }
        }
        else if(strcmp(token, "sacar") == 0) {
            if(strcmp(token_numero, "r_acumulador64") == 0) { fputc(0x58, archivo_escritura) ; }
            else if(strcmp(token_numero, "r_base64") == 0) { fputc(0x5B, archivo_escritura) ; }
        }
        else if(strcmp(token, "terminar") == 0) {
            fputc(0xB8, archivo_escritura) ; fputc(0x3C, archivo_escritura) ; fputc(0x00, archivo_escritura) ; fputc(0x00, archivo_escritura) ; fputc(0x00, archivo_escritura) ;
            fputc(0xBF, archivo_escritura) ; fputc(0x00, archivo_escritura) ; fputc(0x00, archivo_escritura) ; fputc(0x00, archivo_escritura) ; fputc(0x00, archivo_escritura) ;
            fputc(0x0F, archivo_escritura) ; fputc(0x05, archivo_escritura) ;
        }
        else if(strcmp(token, "interrumpir") == 0) {
            fputc(0x0F, archivo_escritura);
            fputc(0x05, archivo_escritura);
        }
        else if(strcmp(token, "saltar") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0xE9, archivo_escritura);
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            }
            else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "saltar_si_igual") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0x0F, archivo_escritura); 
            fputc(0x84, archivo_escritura); // Opcode JE rel32 (32 bits)
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "saltar_si_distinto") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0x0F, archivo_escritura); 
            fputc(0x85, archivo_escritura); // Opcode JNE rel32 (32 bits)
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "saltar_si_mayor") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0x0F, archivo_escritura); fputc(0x8F, archivo_escritura); // Opcode JG rel32
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "saltar_si_menor") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0x0F, archivo_escritura); fputc(0x8C, archivo_escritura); // Opcode JL rel32
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "saltar_si_mayor_o_igual") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0x0F, archivo_escritura); fputc(0x8D, archivo_escritura); // Opcode JGE rel32
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "saltar_si_menor_o_igual") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0x0F, archivo_escritura); fputc(0x8E, archivo_escritura); // Opcode JLE rel32
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "llamar") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            fputc(0xE8, archivo_escritura); // Opcode CALL
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "cargar_direccion") == 0) {
            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, token_numero) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            if(strcmp(token_registros, "r_acumulador64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x8D, archivo_escritura); fputc(0x05, archivo_escritura);
            } else if(strcmp(token_registros, "r_base64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x8D, archivo_escritura); fputc(0x1D, archivo_escritura);
            } else if(strcmp(token_registros, "r_puntero64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x8D, archivo_escritura); fputc(0x35, archivo_escritura);
            } else if(strcmp(token_registros, "r_destino64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x8D, archivo_escritura); fputc(0x3D, archivo_escritura);
            } else {
                continue; 
            }
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);

                

                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, token_numero);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "cargar") == 0) {
            char nombre_etiqueta[50];
            strcpy(nombre_etiqueta, token_numero);
            
            int es_indirecto = 0;
            if (nombre_etiqueta[0] == '[') {
                memmove(nombre_etiqueta, nombre_etiqueta + 1, strlen(nombre_etiqueta));
                nombre_etiqueta[strlen(nombre_etiqueta) - 1] = '\0';
                es_indirecto = 1;
            } else if (strncmp(nombre_etiqueta, "r_", 2) == 0) {
                // Si no tiene corchetes pero es un registro, asumimos que es acceso indirecto
                es_indirecto = 1;
            }

            if (es_indirecto) {
                // --- LÓGICA DE MEMORIA INDIRECTA (Cargar de [registro + offset]) ---
                char base_reg[50] = "";
                long offset = 0;
                char *mas = strchr(nombre_etiqueta, '+');
                char *menos = strchr(nombre_etiqueta, '-');

                if (mas != NULL) {
                    *mas = '\0';
                    strcpy(base_reg, nombre_etiqueta);
                    offset = atoi(mas + 1);
                } else if (menos != NULL) {
                    *menos = '\0';
                    strcpy(base_reg, nombre_etiqueta);
                    offset = -atoi(menos + 1);
                } else {
                    strcpy(base_reg, nombre_etiqueta);
                }

                fputc(0x48, archivo_escritura);
                fputc(0x8B, archivo_escritura); // Opcode MOV reg, r/m64

                int modrm = 0;

                if(strcmp(base_reg, "r_destino64") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) modrm = 0x07;
                    else if(strcmp(token_registros, "r_base64") == 0) modrm = 0x1F;
                    else if(strcmp(token_registros, "r_contador64") == 0) modrm = 0x0F;
                    else if(strcmp(token_registros, "r_datos64") == 0) modrm = 0x17;
                    else if(strcmp(token_registros, "r_puntero64") == 0) modrm = 0x37;
                    else if(strcmp(token_registros, "r_destino64") == 0) modrm = 0x3F;
                    else if(strcmp(token_registros, "r_pila64") == 0) modrm = 0x27;
                }
                else if(strcmp(base_reg, "r_puntero64") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) modrm = 0x06;
                    else if(strcmp(token_registros, "r_base64") == 0) modrm = 0x1E;
                    else if(strcmp(token_registros, "r_contador64") == 0) modrm = 0x0E;
                    else if(strcmp(token_registros, "r_datos64") == 0) modrm = 0x16;
                    else if(strcmp(token_registros, "r_puntero64") == 0) modrm = 0x36;
                    else if(strcmp(token_registros, "r_destino64") == 0) modrm = 0x3E;
                    else if(strcmp(token_registros, "r_pila64") == 0) modrm = 0x26;
                }
                else if(strcmp(base_reg, "r_acumulador64") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) modrm = 0x00;
                    else if(strcmp(token_registros, "r_base64") == 0) modrm = 0x18;
                    else if(strcmp(token_registros, "r_contador64") == 0) modrm = 0x08;
                    else if(strcmp(token_registros, "r_datos64") == 0) modrm = 0x10;
                    else if(strcmp(token_registros, "r_puntero64") == 0) modrm = 0x30;
                    else if(strcmp(token_registros, "r_destino64") == 0) modrm = 0x38;
                    else if(strcmp(token_registros, "r_pila64") == 0) modrm = 0x20;
                }
                else if(strcmp(base_reg, "r_base64") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) modrm = 0x03;
                    else if(strcmp(token_registros, "r_base64") == 0) modrm = 0x1B;
                    else if(strcmp(token_registros, "r_contador64") == 0) modrm = 0x0B;
                    else if(strcmp(token_registros, "r_datos64") == 0) modrm = 0x13;
                    else if(strcmp(token_registros, "r_puntero64") == 0) modrm = 0x33;
                    else if(strcmp(token_registros, "r_destino64") == 0) modrm = 0x3B;
                    else if(strcmp(token_registros, "r_pila64") == 0) modrm = 0x23;
                }
                else if(strcmp(base_reg, "r_contador64") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) modrm = 0x01;
                    else if(strcmp(token_registros, "r_base64") == 0) modrm = 0x19;
                    else if(strcmp(token_registros, "r_contador64") == 0) modrm = 0x09;
                    else if(strcmp(token_registros, "r_datos64") == 0) modrm = 0x11;
                    else if(strcmp(token_registros, "r_puntero64") == 0) modrm = 0x31;
                    else if(strcmp(token_registros, "r_destino64") == 0) modrm = 0x39;
                    else if(strcmp(token_registros, "r_pila64") == 0) modrm = 0x21;
                }
                else if(strcmp(base_reg, "r_datos64") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) modrm = 0x02;
                    else if(strcmp(token_registros, "r_base64") == 0) modrm = 0x1A;
                    else if(strcmp(token_registros, "r_contador64") == 0) modrm = 0x0A;
                    else if(strcmp(token_registros, "r_datos64") == 0) modrm = 0x12;
                    else if(strcmp(token_registros, "r_puntero64") == 0) modrm = 0x32;
                    else if(strcmp(token_registros, "r_destino64") == 0) modrm = 0x3A;
                    else if(strcmp(token_registros, "r_pila64") == 0) modrm = 0x22;
                }
                else if(strcmp(base_reg, "r_pila64") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) modrm = 0x04;
                    else if(strcmp(token_registros, "r_base64") == 0) modrm = 0x1C;
                    else if(strcmp(token_registros, "r_contador64") == 0) modrm = 0x0C;
                    else if(strcmp(token_registros, "r_datos64") == 0) modrm = 0x14;
                    else if(strcmp(token_registros, "r_puntero64") == 0) modrm = 0x34;
                    else if(strcmp(token_registros, "r_destino64") == 0) modrm = 0x3C;
                    else if(strcmp(token_registros, "r_pila64") == 0) modrm = 0x24;
                }

                if (modrm != 0 || strcmp(base_reg, "r_acumulador64") == 0) {
                    if (offset != 0) {
                        fputc(modrm | 0x80, archivo_escritura);
                        fwrite(&offset, sizeof(int), 1, archivo_escritura);
                    } else {
                        fputc(modrm, archivo_escritura);
                    }
                }

            } else {
                // --- LÓGICA DE ETIQUETAS (RIP-relative) ---
                long posicion_destino = -1;
                for (int i = 0; i < num_etiquetas; i++) {
                    if (strcmp(tabla_etiquetas[i].nombre, nombre_etiqueta) == 0) {
                        posicion_destino = tabla_etiquetas[i].posicion;
                        break;
                    }
                }
                
                if(strcmp(token_registros, "r_acumulador64") == 0) {
                    fputc(0x48, archivo_escritura); fputc(0x8B, archivo_escritura); fputc(0x05, archivo_escritura);
                } else if(strcmp(token_registros, "r_base64") == 0) {
                    fputc(0x48, archivo_escritura); fputc(0x8B, archivo_escritura); fputc(0x1D, archivo_escritura);
                }
                
                if (posicion_destino != -1) {
                    long posicion_actual = ftell(archivo_escritura) + 4;
                    int distancia = (int)(posicion_destino - posicion_actual);
                    fwrite(&distancia, sizeof(int), 1, archivo_escritura);
                } else {
                    if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                        strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, nombre_etiqueta);
                        saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                        num_saltos_pendientes++;
                    }
                    int hueco = 0;
                    fwrite(&hueco, sizeof(int), 1, archivo_escritura);
                }
            }
        }
        else if(strcmp(token, "guardar") == 0) {
            char nombre_etiqueta[50];
            strcpy(nombre_etiqueta, token_registros);
            if (nombre_etiqueta[0] == '[') {
                memmove(nombre_etiqueta, nombre_etiqueta + 1, strlen(nombre_etiqueta));
                nombre_etiqueta[strlen(nombre_etiqueta) - 1] = '\0';
            }

            long posicion_destino = -1;
            for (int i = 0; i < num_etiquetas; i++) {
                if (strcmp(tabla_etiquetas[i].nombre, nombre_etiqueta) == 0) {
                    posicion_destino = tabla_etiquetas[i].posicion;
                    break;
                }
            }
            
            if(strcmp(token_numero, "r_acumulador64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0x05, archivo_escritura);
            } else if(strcmp(token_numero, "r_base64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0x1D, archivo_escritura);
            }
            
            if (posicion_destino != -1) {
                long posicion_actual = ftell(archivo_escritura) + 4;
                int distancia = (int)(posicion_destino - posicion_actual);
                fwrite(&distancia, sizeof(int), 1, archivo_escritura);
            } else {
                if (num_saltos_pendientes < MAX_SALTOS_PENDIENTES) {
                    strcpy(saltos_pendientes[num_saltos_pendientes].nombre_etiqueta, nombre_etiqueta);
                    saltos_pendientes[num_saltos_pendientes].posicion_hueco = ftell(archivo_escritura);
                    num_saltos_pendientes++;
                }
                int hueco = 0;
                fwrite(&hueco, sizeof(int), 1, archivo_escritura);
            }
        }
        else if(strcmp(token, "guardar_puntero") == 0) {
            // Sintaxis: guardar_puntero registro_origen, [registro_direccion + offset]
            // token_numero = "r_datos64", token_registros = "[r_destino64+8]"
            
            char reg_dir[50];
            strcpy(reg_dir, token_registros);
            
            // 1. Limpiar corchetes
            if (reg_dir[0] == '[') {
                memmove(reg_dir, reg_dir + 1, strlen(reg_dir));
                reg_dir[strlen(reg_dir) - 1] = '\0';
            }

            // 2. Extraer el registro base y el offset
            char base_reg[50] = "";
            long offset = 0;
            char *mas = strchr(reg_dir, '+');
            char *menos = strchr(reg_dir, '-');

            if (mas != NULL) {
                *mas = '\0';
                strcpy(base_reg, reg_dir);
                offset = atoi(mas + 1);
            } else if (menos != NULL) {
                *menos = '\0';
                strcpy(base_reg, reg_dir);
                offset = -atoi(menos + 1);
            } else {
                strcpy(base_reg, reg_dir);
            }
            
            fputc(0x48, archivo_escritura);
            fputc(0x89, archivo_escritura);
            
            int modrm = 0; // Guardamos el byte ModR/M aquí

            if(strcmp(base_reg, "r_destino64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x07;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1F;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0F;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x17;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x37;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3F;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x27;
            }
            else if(strcmp(base_reg, "r_puntero64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x06;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1E;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0E;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x16;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x36;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3E;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x26;
            }
            else if(strcmp(base_reg, "r_acumulador64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x00;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x18;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x08;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x10;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x30;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x38;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x20;
            }
            else if(strcmp(base_reg, "r_base64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x03;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1B;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0B;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x13;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x33;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3B;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x23;
            }
            else if(strcmp(base_reg, "r_contador64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x01;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x19;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x09;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x11;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x31;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x39;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x21;
            }
            else if(strcmp(base_reg, "r_datos64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x02;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1A;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0A;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x12;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x32;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3A;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x22;
            }
            else if(strcmp(base_reg, "r_pila64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x04;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1C;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0C;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x14;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x34;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3C;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x24;
            }

            // 3. Escribir el ModR/M y el Offset
            if (modrm != 0 || strcmp(base_reg, "r_acumulador64") == 0) { 
                // El check de r_acumulador64 es porque el modrm para [RAX] es 0x00 y chocaría con el valor inicial de modrm
                if (offset != 0) {
                    fputc(modrm | 0x80, archivo_escritura); // Activa el modo [base + disp32]
                    fwrite(&offset, sizeof(int), 1, archivo_escritura);
                } else {
                    fputc(modrm, archivo_escritura); // Sin offset
                }
            }
        }
        
        else if(strcmp(token, "cargar_doble_puntero") == 0) {
            char reg_dir[50];
            strcpy(reg_dir, token_numero); // Aquí está la memoria [r_base64+4]
            if (reg_dir[0] == '[') {
                memmove(reg_dir, reg_dir + 1, strlen(reg_dir));
                reg_dir[strlen(reg_dir) - 1] = '\0';
            }
            
            char base_reg[50] = "";
            long offset = 0;
            char *mas = strchr(reg_dir, '+');
            char *menos = strchr(reg_dir, '-');

            if (mas != NULL) { *mas = '\0'; strcpy(base_reg, reg_dir); offset = atoi(mas + 1); }
            else if (menos != NULL) { *menos = '\0'; strcpy(base_reg, reg_dir); offset = -atoi(menos + 1); }
            else { strcpy(base_reg, reg_dir); }
            
            int modrm = 0;

            // Base: r_acumulador64 (RAX)
            if(strcmp(base_reg, "r_acumulador64") == 0) {
                if(strcmp(token_registros, "r_acumulador32") == 0) modrm = 0x00;
                else if(strcmp(token_registros, "r_contador32") == 0) modrm = 0x08;
                else if(strcmp(token_registros, "r_datos32") == 0) modrm = 0x10;
                else if(strcmp(token_registros, "r_base32") == 0) modrm = 0x18;
                else if(strcmp(token_registros, "r_puntero32") == 0) modrm = 0x30;
                else if(strcmp(token_registros, "r_destino32") == 0) modrm = 0x38;
            }
            // Base: r_contador64 (RCX)
            else if(strcmp(base_reg, "r_contador64") == 0) {
                if(strcmp(token_registros, "r_acumulador32") == 0) modrm = 0x01;
                else if(strcmp(token_registros, "r_contador32") == 0) modrm = 0x09;
                else if(strcmp(token_registros, "r_datos32") == 0) modrm = 0x11;
                else if(strcmp(token_registros, "r_base32") == 0) modrm = 0x19;
                else if(strcmp(token_registros, "r_puntero32") == 0) modrm = 0x31;
                else if(strcmp(token_registros, "r_destino32") == 0) modrm = 0x39;
            }
            // Base: r_datos64 (RDX)
            else if(strcmp(base_reg, "r_datos64") == 0) {
                if(strcmp(token_registros, "r_acumulador32") == 0) modrm = 0x02;
                else if(strcmp(token_registros, "r_contador32") == 0) modrm = 0x0A;
                else if(strcmp(token_registros, "r_datos32") == 0) modrm = 0x12;
                else if(strcmp(token_registros, "r_base32") == 0) modrm = 0x1A;
                else if(strcmp(token_registros, "r_puntero32") == 0) modrm = 0x32;
                else if(strcmp(token_registros, "r_destino32") == 0) modrm = 0x3A;
            }
            // Base: r_base64 (RBX)
            else if(strcmp(base_reg, "r_base64") == 0) {
                if(strcmp(token_registros, "r_acumulador32") == 0) modrm = 0x03;
                else if(strcmp(token_registros, "r_contador32") == 0) modrm = 0x0B;
                else if(strcmp(token_registros, "r_datos32") == 0) modrm = 0x13;
                else if(strcmp(token_registros, "r_base32") == 0) modrm = 0x1B;
                else if(strcmp(token_registros, "r_puntero32") == 0) modrm = 0x33;
                else if(strcmp(token_registros, "r_destino32") == 0) modrm = 0x3B;
            }
            // Base: r_puntero64 (RSI)
            else if(strcmp(base_reg, "r_puntero64") == 0) {
                if(strcmp(token_registros, "r_acumulador32") == 0) modrm = 0x06;
                else if(strcmp(token_registros, "r_contador32") == 0) modrm = 0x0E;
                else if(strcmp(token_registros, "r_datos32") == 0) modrm = 0x16;
                else if(strcmp(token_registros, "r_base32") == 0) modrm = 0x1E;
                else if(strcmp(token_registros, "r_puntero32") == 0) modrm = 0x36;
                else if(strcmp(token_registros, "r_destino32") == 0) modrm = 0x3E;
            }
            // Base: r_destino64 (RDI)
            else if(strcmp(base_reg, "r_destino64") == 0) {
                if(strcmp(token_registros, "r_acumulador32") == 0) modrm = 0x07;
                else if(strcmp(token_registros, "r_contador32") == 0) modrm = 0x0F;
                else if(strcmp(token_registros, "r_datos32") == 0) modrm = 0x17;
                else if(strcmp(token_registros, "r_base32") == 0) modrm = 0x1F;
                else if(strcmp(token_registros, "r_puntero32") == 0) modrm = 0x37;
                else if(strcmp(token_registros, "r_destino32") == 0) modrm = 0x3F;
            }

            if (modrm != 0 || strcmp(base_reg, "r_acumulador64") == 0) {
                fputc(0x8B, archivo_escritura); // Opcode MOV r32, r/m32
                if (offset != 0) {
                    fputc(modrm | 0x80, archivo_escritura);
                    fwrite(&offset, sizeof(int), 1, archivo_escritura);
                } else {
                    fputc(modrm, archivo_escritura);
                }
            }
        }
        else if(strcmp(token, "guardar_byte_puntero") == 0) {
            char reg_dir[50];
            strcpy(reg_dir, token_registros);
            if (reg_dir[0] == '[') {
                memmove(reg_dir, reg_dir + 1, strlen(reg_dir));
                reg_dir[strlen(reg_dir) - 1] = '\0';
            }
            
            char base_reg[50] = "";
            long offset = 0;
            char *mas = strchr(reg_dir, '+');
            char *menos = strchr(reg_dir, '-');

            if (mas != NULL) {
                *mas = '\0'; strcpy(base_reg, reg_dir); offset = atoi(mas + 1);
            } else if (menos != NULL) {
                *menos = '\0'; strcpy(base_reg, reg_dir); offset = -atoi(menos + 1);
            } else {
                strcpy(base_reg, reg_dir);
            }
            
            int modrm = 0;
            if(strcmp(base_reg, "r_destino64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x07;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1F;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0F;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x17;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x37;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3F;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x27;
            }
            else if(strcmp(base_reg, "r_puntero64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x06;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1E;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0E;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x16;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x36;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3E;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x26;
            }
            else if(strcmp(base_reg, "r_base64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x03;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1B;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0B;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x13;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x33;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3B;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x23;
            }
            else if(strcmp(base_reg, "r_acumulador64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x00;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x18;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x08;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x10;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x30;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x38;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x20;
            }
            else if(strcmp(base_reg, "r_contador64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x01;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x19;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x09;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x11;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x31;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x39;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x21;
            }
            else if(strcmp(base_reg, "r_datos64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x02;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1A;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0A;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x12;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x32;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3A;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x22;
            }
            else if(strcmp(base_reg, "r_pila64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) modrm = 0x04;
                else if(strcmp(token_numero, "r_base64") == 0) modrm = 0x1C;
                else if(strcmp(token_numero, "r_contador64") == 0) modrm = 0x0C;
                else if(strcmp(token_numero, "r_datos64") == 0) modrm = 0x14;
                else if(strcmp(token_numero, "r_puntero64") == 0) modrm = 0x34;
                else if(strcmp(token_numero, "r_destino64") == 0) modrm = 0x3C;
                else if(strcmp(token_numero, "r_pila64") == 0) modrm = 0x24;
            }

            if (modrm != 0 || strcmp(base_reg, "r_acumulador64") == 0) {
                fputc(0x88, archivo_escritura); // Opcode MOV byte
                if (offset != 0) {
                    fputc(modrm | 0x80, archivo_escritura); // Activa [base + disp32]
                    fwrite(&offset, sizeof(int), 1, archivo_escritura);
                } else {
                    fputc(modrm, archivo_escritura); // Sin offset
                }
            }
        }
        else if(strcmp(token, "guardar_doble_puntero") == 0) {
            char reg_dir[50];
            strcpy(reg_dir, token_registros); // La memoria [r_base64+4] está en token_registros
            if (reg_dir[0] == '[') {
                memmove(reg_dir, reg_dir + 1, strlen(reg_dir));
                reg_dir[strlen(reg_dir) - 1] = '\0';
            }
            
            char base_reg[50] = "";
            long offset = 0;
            char *mas = strchr(reg_dir, '+');
            char *menos = strchr(reg_dir, '-');

            if (mas != NULL) { *mas = '\0'; strcpy(base_reg, reg_dir); offset = atoi(mas + 1); }
            else if (menos != NULL) { *menos = '\0'; strcpy(base_reg, reg_dir); offset = -atoi(menos + 1); }
            else { strcpy(base_reg, reg_dir); }
            
            int modrm = 0;

            // Base: r_acumulador64 (RAX)
            if(strcmp(base_reg, "r_acumulador64") == 0) {
                if(strcmp(token_numero, "r_acumulador32") == 0) modrm = 0x00;
                else if(strcmp(token_numero, "r_contador32") == 0) modrm = 0x08;
                else if(strcmp(token_numero, "r_datos32") == 0) modrm = 0x10;
                else if(strcmp(token_numero, "r_base32") == 0) modrm = 0x18;
                else if(strcmp(token_numero, "r_puntero32") == 0) modrm = 0x30;
                else if(strcmp(token_numero, "r_destino32") == 0) modrm = 0x38;
            }
            // Base: r_contador64 (RCX)
            else if(strcmp(base_reg, "r_contador64") == 0) {
                if(strcmp(token_numero, "r_acumulador32") == 0) modrm = 0x01;
                else if(strcmp(token_numero, "r_contador32") == 0) modrm = 0x09;
                else if(strcmp(token_numero, "r_datos32") == 0) modrm = 0x11;
                else if(strcmp(token_numero, "r_base32") == 0) modrm = 0x19;
                else if(strcmp(token_numero, "r_puntero32") == 0) modrm = 0x31;
                else if(strcmp(token_numero, "r_destino32") == 0) modrm = 0x39;
            }
            // Base: r_datos64 (RDX)
            else if(strcmp(base_reg, "r_datos64") == 0) {
                if(strcmp(token_numero, "r_acumulador32") == 0) modrm = 0x02;
                else if(strcmp(token_numero, "r_contador32") == 0) modrm = 0x0A;
                else if(strcmp(token_numero, "r_datos32") == 0) modrm = 0x12;
                else if(strcmp(token_numero, "r_base32") == 0) modrm = 0x1A;
                else if(strcmp(token_numero, "r_puntero32") == 0) modrm = 0x32;
                else if(strcmp(token_numero, "r_destino32") == 0) modrm = 0x3A;
            }
            // Base: r_base64 (RBX)
            else if(strcmp(base_reg, "r_base64") == 0) {
                if(strcmp(token_numero, "r_acumulador32") == 0) modrm = 0x03;
                else if(strcmp(token_numero, "r_contador32") == 0) modrm = 0x0B;
                else if(strcmp(token_numero, "r_datos32") == 0) modrm = 0x13;
                else if(strcmp(token_numero, "r_base32") == 0) modrm = 0x1B;
                else if(strcmp(token_numero, "r_puntero32") == 0) modrm = 0x33;
                else if(strcmp(token_numero, "r_destino32") == 0) modrm = 0x3B;
            }
            // Base: r_puntero64 (RSI)
            else if(strcmp(base_reg, "r_puntero64") == 0) {
                if(strcmp(token_numero, "r_acumulador32") == 0) modrm = 0x06;
                else if(strcmp(token_numero, "r_contador32") == 0) modrm = 0x0E;
                else if(strcmp(token_numero, "r_datos32") == 0) modrm = 0x16;
                else if(strcmp(token_numero, "r_base32") == 0) modrm = 0x1E;
                else if(strcmp(token_numero, "r_puntero32") == 0) modrm = 0x36;
                else if(strcmp(token_numero, "r_destino32") == 0) modrm = 0x3E;
            }
            // Base: r_destino64 (RDI)
            else if(strcmp(base_reg, "r_destino64") == 0) {
                if(strcmp(token_numero, "r_acumulador32") == 0) modrm = 0x07;
                else if(strcmp(token_numero, "r_contador32") == 0) modrm = 0x0F;
                else if(strcmp(token_numero, "r_datos32") == 0) modrm = 0x17;
                else if(strcmp(token_numero, "r_base32") == 0) modrm = 0x1F;
                else if(strcmp(token_numero, "r_puntero32") == 0) modrm = 0x37;
                else if(strcmp(token_numero, "r_destino32") == 0) modrm = 0x3F;
            }

            if (modrm != 0 || strcmp(base_reg, "r_acumulador64") == 0) {
                fputc(0x89, archivo_escritura); // Opcode MOV r/m32, r32
                if (offset != 0) {
                    fputc(modrm | 0x80, archivo_escritura);
                    fwrite(&offset, sizeof(int), 1, archivo_escritura);
                } else {
                    fputc(modrm, archivo_escritura);
                }
            }
        }
        else if(strcmp(token, "guardar_indirecto") == 0) {
            // Sintaxis: guardar_indirecto registro_origen, [registro_direccion]
            // token_numero = "r_acumulador64", token_registros = "[r_puntero64]"
            
            char reg_dir[50];
            strcpy(reg_dir, token_registros);
            if (reg_dir[0] == '[') {
                memmove(reg_dir, reg_dir + 1, strlen(reg_dir));
                reg_dir[strlen(reg_dir) - 1] = '\0';
            }
            
            fputc(0x48, archivo_escritura);
            fputc(0x89, archivo_escritura);
            
            if(strcmp(reg_dir, "r_puntero64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) fputc(0x06, archivo_escritura);
                else if(strcmp(token_numero, "r_base64") == 0) fputc(0x1E, archivo_escritura);
                else if(strcmp(token_numero, "r_contador64") == 0) fputc(0x0E, archivo_escritura);
                else if(strcmp(token_numero, "r_datos64") == 0) fputc(0x16, archivo_escritura);
                else if(strcmp(token_numero, "r_puntero64") == 0) fputc(0x36, archivo_escritura);
                else if(strcmp(token_numero, "r_destino64") == 0) fputc(0x3E, archivo_escritura);
                else if(strcmp(token_numero, "r_pila64") == 0) fputc(0x26, archivo_escritura);
            }
            else if(strcmp(reg_dir, "r_destino64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) fputc(0x07, archivo_escritura);
                else if(strcmp(token_numero, "r_base64") == 0) fputc(0x1F, archivo_escritura);
                else if(strcmp(token_numero, "r_contador64") == 0) fputc(0x0F, archivo_escritura);
                else if(strcmp(token_numero, "r_datos64") == 0) fputc(0x17, archivo_escritura);
                else if(strcmp(token_numero, "r_puntero64") == 0) fputc(0x37, archivo_escritura);
                else if(strcmp(token_numero, "r_destino64") == 0) fputc(0x3F, archivo_escritura);
                else if(strcmp(token_numero, "r_pila64") == 0) fputc(0x27, archivo_escritura);
            }
            else if(strcmp(reg_dir, "r_acumulador64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) fputc(0x00, archivo_escritura);
                else if(strcmp(token_numero, "r_base64") == 0) fputc(0x18, archivo_escritura);
                else if(strcmp(token_numero, "r_contador64") == 0) fputc(0x08, archivo_escritura);
                else if(strcmp(token_numero, "r_datos64") == 0) fputc(0x10, archivo_escritura);
                else if(strcmp(token_numero, "r_puntero64") == 0) fputc(0x30, archivo_escritura);
                else if(strcmp(token_numero, "r_destino64") == 0) fputc(0x38, archivo_escritura);
                else if(strcmp(token_numero, "r_pila64") == 0) fputc(0x20, archivo_escritura);
            }
            else if(strcmp(reg_dir, "r_base64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) fputc(0x03, archivo_escritura);
                else if(strcmp(token_numero, "r_base64") == 0) fputc(0x1B, archivo_escritura);
                else if(strcmp(token_numero, "r_contador64") == 0) fputc(0x0B, archivo_escritura);
                else if(strcmp(token_numero, "r_datos64") == 0) fputc(0x13, archivo_escritura);
                else if(strcmp(token_numero, "r_puntero64") == 0) fputc(0x33, archivo_escritura);
                else if(strcmp(token_numero, "r_destino64") == 0) fputc(0x3B, archivo_escritura);
                else if(strcmp(token_numero, "r_pila64") == 0) fputc(0x23, archivo_escritura);
            }
            else if(strcmp(reg_dir, "r_contador64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) fputc(0x01, archivo_escritura);
                else if(strcmp(token_numero, "r_base64") == 0) fputc(0x19, archivo_escritura);
                else if(strcmp(token_numero, "r_contador64") == 0) fputc(0x09, archivo_escritura);
                else if(strcmp(token_numero, "r_datos64") == 0) fputc(0x11, archivo_escritura);
                else if(strcmp(token_numero, "r_puntero64") == 0) fputc(0x31, archivo_escritura);
                else if(strcmp(token_numero, "r_destino64") == 0) fputc(0x39, archivo_escritura);
                else if(strcmp(token_numero, "r_pila64") == 0) fputc(0x21, archivo_escritura);
            }
            else if(strcmp(reg_dir, "r_datos64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) fputc(0x02, archivo_escritura);
                else if(strcmp(token_numero, "r_base64") == 0) fputc(0x1A, archivo_escritura);
                else if(strcmp(token_numero, "r_contador64") == 0) fputc(0x0A, archivo_escritura);
                else if(strcmp(token_numero, "r_datos64") == 0) fputc(0x12, archivo_escritura);
                else if(strcmp(token_numero, "r_puntero64") == 0) fputc(0x32, archivo_escritura);
                else if(strcmp(token_numero, "r_destino64") == 0) fputc(0x3A, archivo_escritura);
                else if(strcmp(token_numero, "r_pila64") == 0) fputc(0x22, archivo_escritura);
            }
            else if(strcmp(reg_dir, "r_pila64") == 0) {
                if(strcmp(token_numero, "r_acumulador64") == 0) fputc(0x04, archivo_escritura);
                else if(strcmp(token_numero, "r_base64") == 0) fputc(0x1C, archivo_escritura);
                else if(strcmp(token_numero, "r_contador64") == 0) fputc(0x0C, archivo_escritura);
                else if(strcmp(token_numero, "r_datos64") == 0) fputc(0x14, archivo_escritura);
                else if(strcmp(token_numero, "r_puntero64") == 0) fputc(0x34, archivo_escritura);
                else if(strcmp(token_numero, "r_destino64") == 0) fputc(0x3C, archivo_escritura);
                else if(strcmp(token_numero, "r_pila64") == 0) fputc(0x24, archivo_escritura);
            }
        }
        else if(strcmp(token, "desplazar_derecha") == 0) {
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEF, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEC, archivo_escritura); }
                
                int num = atoi(token_numero); 
                fputc(num, archivo_escritura); 
            }
            else {
                // MODO REGISTRO A REGISTRO (Usando CL)
                if(strcmp(token_numero, "r_contador8") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE8, archivo_escritura); } // SHR RAX, CL
                    else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xEB, archivo_escritura); } // SHR RBX, CL
                    else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE9, archivo_escritura); } // SHR RCX, CL
                    else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xEA, archivo_escritura); } // SHR RDX, CL
                    else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xEE, archivo_escritura); } // SHR RSI, CL
                    else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xEF, archivo_escritura); } // SHR RDI, CL
                    else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xEC, archivo_escritura); } // SHR RSP, CL
                }
            }
        }
        else if(strcmp(token, "desplazar_izquierda") == 0) {
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE4, archivo_escritura); }
                
                int num = atoi(token_numero); 
                fputc(num, archivo_escritura); 
            }
            else {
                // MODO REGISTRO A REGISTRO (Usando CL)
                if(strcmp(token_numero, "r_contador8") == 0) {
                    if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE0, archivo_escritura); } // SHL RAX, CL
                    else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE3, archivo_escritura); } // SHL RBX, CL
                    else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE1, archivo_escritura); } // SHL RCX, CL
                    else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE2, archivo_escritura); } // SHL RDX, CL
                    else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE6, archivo_escritura); } // SHL RSI, CL
                    else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE7, archivo_escritura); } // SHL RDI, CL
                    else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0xD3, archivo_escritura); fputc(0xE4, archivo_escritura); } // SHL RSP, CL
                }
            }
        }
                else if(strcmp(token, "y") == 0) {
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x25, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE4, archivo_escritura); }
                
                int numero = atoi(token_numero);
                fwrite(&numero, sizeof(int), 1, archivo_escritura);
            }
            else {
                // MODO REGISTRO A REGISTRO
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xE0, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xDB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xCB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xFB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xE3, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xE1, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xDA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xCA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xFA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xE2, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xDE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xCE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xFE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xE6, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xDF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xCF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xFF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xE7, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_pila64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xC4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xDC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xCC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xD4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xF4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xFC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x21, archivo_escritura); fputc(0xE4, archivo_escritura); }
                }
            }
        }
        else if(strcmp(token, "o") == 0) {
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x0D, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCF, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCC, archivo_escritura); }
                
                int numero = atoi(token_numero);
                fwrite(&numero, sizeof(int), 1, archivo_escritura);
            }
            else {
                // MODO REGISTRO A REGISTRO
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xE0, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xDB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xCB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xFB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xE3, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xE1, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xDA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xCA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xFA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xE2, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xDE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xCE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xFE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xE6, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xDF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xCF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xFF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xE7, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_pila64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xC4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xDC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xCC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xD4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xF4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xFC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x09, archivo_escritura); fputc(0xE4, archivo_escritura); }
                }
            }
        }
        else if(strcmp(token, "sumar") == 0) {
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador32") == 0) {
                    fputc(0x05, archivo_escritura) ;
                }
                else if(strcmp(token_registros, "r_base32") == 0) {
                    fputc(0x81, archivo_escritura) ;
                    fputc(0xC3, archivo_escritura) ;
                } 
                else if(strcmp(token_registros, "r_base64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xC3, archivo_escritura) ; 
                    es_64 = 1 ;
                }
                else if(strcmp(token_registros, "r_acumulador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x05, archivo_escritura) ;
                    es_64 = 1 ;
                }
                else if(strcmp(token_registros, "r_contador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xC1, archivo_escritura) ; // ADD RCX, imm32
                }
                else if(strcmp(token_registros, "r_datos64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xC2, archivo_escritura) ; // ADD RDX, imm32
                }
                else if(strcmp(token_registros, "r_puntero64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xC6, archivo_escritura) ; // ADD RSI, imm32
                }
                else if(strcmp(token_registros, "r_destino64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xC7, archivo_escritura) ; // ADD RDI, imm32
                }
                else if(strcmp(token_registros, "r_pila64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xC4, archivo_escritura) ; // ADD RSP, imm32
                }
                
                int numero = atoi(token_numero) ;
                fwrite(&numero, sizeof(int), 1, archivo_escritura) ;
            }
            else {
                // MODO REGISTRO A REGISTRO
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD8, archivo_escritura); } // ADD RAX, RBX
                    else if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC0, archivo_escritura); } // ADD RAX, RAX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC8, archivo_escritura); } // ADD RAX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD0, archivo_escritura); } // ADD RAX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF0, archivo_escritura); } // ADD RAX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF8, archivo_escritura); } // ADD RAX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xE0, archivo_escritura); } // ADD RAX, RSP
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC3, archivo_escritura); } // ADD RBX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xDB, archivo_escritura); } // ADD RBX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xCB, archivo_escritura); } // ADD RBX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD3, archivo_escritura); } // ADD RBX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF3, archivo_escritura); } // ADD RBX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xFB, archivo_escritura); } // ADD RBX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xE3, archivo_escritura); } // ADD RBX, RSP
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC1, archivo_escritura); } // ADD RCX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD9, archivo_escritura); } // ADD RCX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC9, archivo_escritura); } // ADD RCX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD1, archivo_escritura); } // ADD RCX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF1, archivo_escritura); } // ADD RCX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF9, archivo_escritura); } // ADD RCX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xE1, archivo_escritura); } // ADD RCX, RSP
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC2, archivo_escritura); } // ADD RDX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xDA, archivo_escritura); } // ADD RDX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xCA, archivo_escritura); } // ADD RDX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD2, archivo_escritura); } // ADD RDX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF2, archivo_escritura); } // ADD RDX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xFA, archivo_escritura); } // ADD RDX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xE2, archivo_escritura); } // ADD RDX, RSP
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC6, archivo_escritura); } // ADD RSI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xDE, archivo_escritura); } // ADD RSI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xCE, archivo_escritura); } // ADD RSI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD6, archivo_escritura); } // ADD RSI, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF6, archivo_escritura); } // ADD RSI, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xFE, archivo_escritura); } // ADD RSI, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xE6, archivo_escritura); } // ADD RSI, RSP
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC7, archivo_escritura); } // ADD RDI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xDF, archivo_escritura); } // ADD RDI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xCF, archivo_escritura); } // ADD RDI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD7, archivo_escritura); } // ADD RDI, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF7, archivo_escritura); } // ADD RDI, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xFF, archivo_escritura); } // ADD RDI, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xE7, archivo_escritura); } // ADD RDI, RSP
                }
                else if(strcmp(token_destino, "r_pila64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xC4, archivo_escritura); } // ADD RSP, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xDC, archivo_escritura); } // ADD RSP, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xCC, archivo_escritura); } // ADD RSP, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xD4, archivo_escritura); } // ADD RSP, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xF4, archivo_escritura); } // ADD RSP, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xFC, archivo_escritura); } // ADD RSP, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x01, archivo_escritura); fputc(0xE4, archivo_escritura); } // ADD RSP, RSP
                }
            }
        }
        else if(strcmp(token, "o_exclusivo") == 0) {
            if(!isdigit(token_numero[0])) {
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF0, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF8, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xE0, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xDB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xCB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF3, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xFB, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xE3, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF1, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF9, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xE1, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xDA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xCA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF2, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xFA, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xE2, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xDE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xCE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF6, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xFE, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xE6, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xDF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xCF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF7, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xFF, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xE7, archivo_escritura); }
                }
                else if(strcmp(token_destino, "r_pila64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xC4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xDC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xCC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xF4, archivo_escritura); }
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xFC, archivo_escritura); }
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xE4, archivo_escritura); }
                }

                // --- NUEVOS REGISTROS DE 32 BITS ---
                else if(strcmp(token_destino, "r_acumulador32") == 0) {
                    if(strcmp(token_origen, "r_acumulador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC0, archivo_escritura); } // XOR EAX, EAX
                    else if(strcmp(token_origen, "r_base32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD8, archivo_escritura); } // XOR EAX, EBX
                    else if(strcmp(token_origen, "r_contador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC8, archivo_escritura); } // XOR EAX, ECX
                    else if(strcmp(token_origen, "r_datos32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD0, archivo_escritura); } // XOR EAX, EDX
                    else if(strcmp(token_origen, "r_puntero32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF0, archivo_escritura); } // XOR EAX, ESI
                    else if(strcmp(token_origen, "r_destino32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF8, archivo_escritura); } // XOR EAX, EDI
                }
                else if(strcmp(token_destino, "r_base32") == 0) {
                    if(strcmp(token_origen, "r_acumulador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC3, archivo_escritura); } // XOR EBX, EAX
                    else if(strcmp(token_origen, "r_base32") == 0) { fputc(0x31, archivo_escritura); fputc(0xDB, archivo_escritura); } // XOR EBX, EBX
                    else if(strcmp(token_origen, "r_contador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xCB, archivo_escritura); } // XOR EBX, ECX
                    else if(strcmp(token_origen, "r_datos32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD3, archivo_escritura); } // XOR EBX, EDX
                    else if(strcmp(token_origen, "r_puntero32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF3, archivo_escritura); } // XOR EBX, ESI
                    else if(strcmp(token_origen, "r_destino32") == 0) { fputc(0x31, archivo_escritura); fputc(0xFB, archivo_escritura); } // XOR EBX, EDI
                }
                else if(strcmp(token_destino, "r_contador32") == 0) {
                    if(strcmp(token_origen, "r_acumulador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC1, archivo_escritura); } // XOR ECX, EAX
                    else if(strcmp(token_origen, "r_base32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD9, archivo_escritura); } // XOR ECX, EBX
                    else if(strcmp(token_origen, "r_contador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC9, archivo_escritura); } // XOR ECX, ECX
                    else if(strcmp(token_origen, "r_datos32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD1, archivo_escritura); } // XOR ECX, EDX
                    else if(strcmp(token_origen, "r_puntero32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF1, archivo_escritura); } // XOR ECX, ESI
                    else if(strcmp(token_origen, "r_destino32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF9, archivo_escritura); } // XOR ECX, EDI
                }
                else if(strcmp(token_destino, "r_datos32") == 0) {
                    if(strcmp(token_origen, "r_acumulador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC2, archivo_escritura); } // XOR EDX, EAX
                    else if(strcmp(token_origen, "r_base32") == 0) { fputc(0x31, archivo_escritura); fputc(0xDA, archivo_escritura); } // XOR EDX, EBX
                    else if(strcmp(token_origen, "r_contador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xCA, archivo_escritura); } // XOR EDX, ECX
                    else if(strcmp(token_origen, "r_datos32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); } // XOR EDX, EDX
                    else if(strcmp(token_origen, "r_puntero32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF2, archivo_escritura); } // XOR EDX, ESI
                    else if(strcmp(token_origen, "r_destino32") == 0) { fputc(0x31, archivo_escritura); fputc(0xFA, archivo_escritura); } // XOR EDX, EDI
                }
                else if(strcmp(token_destino, "r_puntero32") == 0) {
                    if(strcmp(token_origen, "r_acumulador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC6, archivo_escritura); } // XOR ESI, EAX
                    else if(strcmp(token_origen, "r_base32") == 0) { fputc(0x31, archivo_escritura); fputc(0xDE, archivo_escritura); } // XOR ESI, EBX
                    else if(strcmp(token_origen, "r_contador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xCE, archivo_escritura); } // XOR ESI, ECX
                    else if(strcmp(token_origen, "r_datos32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD6, archivo_escritura); } // XOR ESI, EDX
                    else if(strcmp(token_origen, "r_puntero32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF6, archivo_escritura); } // XOR ESI, ESI
                    else if(strcmp(token_origen, "r_destino32") == 0) { fputc(0x31, archivo_escritura); fputc(0xFE, archivo_escritura); } // XOR ESI, EDI
                }
                else if(strcmp(token_destino, "r_destino32") == 0) {
                    if(strcmp(token_origen, "r_acumulador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xC7, archivo_escritura); } // XOR EDI, EAX
                    else if(strcmp(token_origen, "r_base32") == 0) { fputc(0x31, archivo_escritura); fputc(0xDF, archivo_escritura); } // XOR EDI, EBX
                    else if(strcmp(token_origen, "r_contador32") == 0) { fputc(0x31, archivo_escritura); fputc(0xCF, archivo_escritura); } // XOR EDI, ECX
                    else if(strcmp(token_origen, "r_datos32") == 0) { fputc(0x31, archivo_escritura); fputc(0xD7, archivo_escritura); } // XOR EDI, EDX
                    else if(strcmp(token_origen, "r_puntero32") == 0) { fputc(0x31, archivo_escritura); fputc(0xF7, archivo_escritura); } // XOR EDI, ESI
                    else if(strcmp(token_origen, "r_destino32") == 0) { fputc(0x31, archivo_escritura); fputc(0xFF, archivo_escritura); } // XOR EDI, EDI
                }
            }
        }

        else if(strcmp(token, "copiar") == 0) {
            if (isdigit(token_numero[0])) { 
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador32") == 0) {
                    fputc(0xB8, archivo_escritura) ; // MOV EAX
                } 
                else if(strcmp(token_registros, "r_base32") == 0) {
                    fputc(0xBB, archivo_escritura) ; // MOV EBX
                }
                else if(strcmp(token_registros, "r_acumulador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0xB8, archivo_escritura) ;  // MOV RAX, imm64
                    es_64 = 1 ;
                }
                else if(strcmp(token_registros, "r_base64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0xBB, archivo_escritura) ; // MOV RBX, imm64
                    es_64 = 1 ; 
                }
                else if(strcmp(token_registros, "r_contador64") == 0) { 
                    fputc(0x48, archivo_escritura) ; fputc(0xB9, archivo_escritura) ; // MOV RCX, imm64
                    es_64 = 1 ; 
                }
                else if(strcmp(token_registros, "r_datos64") == 0) { 
                    fputc(0x48, archivo_escritura) ; fputc(0xBA, archivo_escritura) ; // MOV RDX, imm64
                    es_64 = 1 ; 
                }
                else if(strcmp(token_registros, "r_puntero64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0xBE, archivo_escritura) ; // MOV RSI, imm64
                    es_64 = 1 ; // <--- AÑADIDO
                }
                else if(strcmp(token_registros, "r_destino64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0xBF, archivo_escritura) ; // MOV RDI, imm64
                    es_64 = 1 ; // <--- AÑADIDO
                }
                
                if(es_64) {
                    long long numerob = atoll(token_numero) ;
                    fwrite(&numerob, sizeof(long long), 1, archivo_escritura) ;
                }
                else {
                    int numero = atoi(token_numero) ;
                    fwrite(&numero, sizeof(int), 1, archivo_escritura) ;
                }
            }
            else {
                // MODO REGISTRO A REGISTRO
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xD8, archivo_escritura); } // MOV RAX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xC8, archivo_escritura); } // MOV RAX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xD0, archivo_escritura); } // MOV RAX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xF0, archivo_escritura); } // MOV RAX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xF8, archivo_escritura); } // MOV RAX, RDI
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xC3, archivo_escritura); } // MOV RBX, RAX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xCB, archivo_escritura); } // MOV RBX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xD3, archivo_escritura); } // MOV RBX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xF3, archivo_escritura); } // MOV RBX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xFB, archivo_escritura); } // MOV RBX, RDI
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xC1, archivo_escritura); } // MOV RCX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xD9, archivo_escritura); } // MOV RCX, RBX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xD1, archivo_escritura); } // MOV RCX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xF1, archivo_escritura); } // MOV RCX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xF9, archivo_escritura); } // MOV RCX, RDI
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xC2, archivo_escritura); } // MOV RDX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xDA, archivo_escritura); } // MOV RDX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xCA, archivo_escritura); } // MOV RDX, RCX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xF2, archivo_escritura); } // MOV RDX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xFA, archivo_escritura); } // MOV RDX, RDI
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xC6, archivo_escritura); } // MOV RSI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xDE, archivo_escritura); } // MOV RSI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xCE, archivo_escritura); } // MOV RSI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xD6, archivo_escritura); } // MOV RSI, RDX
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xFE, archivo_escritura); } // MOV RSI, RDI
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xC7, archivo_escritura); } // MOV RDI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xDF, archivo_escritura); } // MOV RDI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xCF, archivo_escritura); } // MOV RDI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xD7, archivo_escritura); } // MOV RDI, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xF7, archivo_escritura); } // MOV RDI, RSI
                }
                else if(strcmp(token_origen, "r_pila64") == 0) {
                    if(strcmp(token_destino, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xE0, archivo_escritura); } // MOV RAX, RSP
                    else if(strcmp(token_destino, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xE3, archivo_escritura); } // MOV RBX, RSP
                    else if(strcmp(token_destino, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xE1, archivo_escritura); } // MOV RCX, RSP
                    else if(strcmp(token_destino, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xE2, archivo_escritura); } // MOV RDX, RSP
                    else if(strcmp(token_destino, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xE6, archivo_escritura); } // MOV RSI, RSP
                    else if(strcmp(token_destino, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x89, archivo_escritura); fputc(0xE7, archivo_escritura); } // MOV RDI, RSP
                }
            } 
        } // <--- ESTA LLAVE CIERRA EL BLOQUE COPIAR

        
        else if(strcmp(token, "restar") == 0) {
            if(isdigit(token_numero[0])) {
                if(strcmp(token_registros, "r_acumulador32") == 0) {
                    fputc(0x2D, archivo_escritura) ;
                }
                else if(strcmp(token_registros, "r_base32") == 0) {
                    fputc(0x81, archivo_escritura) ;
                    fputc(0xEB, archivo_escritura) ;
                }
                else if(strcmp(token_registros, "r_acumulador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x2D, archivo_escritura) ;
                    es_64 = 1 ;
                }
                else if(strcmp(token_registros, "r_base64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xEB, archivo_escritura) ;
                    es_64 = 1 ;
                }
                else if(strcmp(token_registros, "r_contador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xE9, archivo_escritura) ; // SUB RCX, imm32
                }
                else if(strcmp(token_registros, "r_datos64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xEA, archivo_escritura) ; // SUB RDX, imm32
                }
                else if(strcmp(token_registros, "r_puntero64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xEE, archivo_escritura) ; // SUB RSI, imm32
                }
                else if(strcmp(token_registros, "r_destino64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xEF, archivo_escritura) ; // SUB RDI, imm32
                }
                else if(strcmp(token_registros, "r_pila64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xEC, archivo_escritura) ; // SUB RSP, imm32
                }
                
                int numero = atoi(token_numero) ;
                fwrite(&numero, sizeof(int), 1, archivo_escritura) ;
            }
            else {
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC0, archivo_escritura) ; } // SUB RAX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD8, archivo_escritura) ; } // SUB RAX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC8, archivo_escritura) ; } // SUB RAX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD0, archivo_escritura) ; } // SUB RAX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF0, archivo_escritura) ; } // SUB RAX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF8, archivo_escritura) ; } // SUB RAX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xE0, archivo_escritura) ; } // SUB RAX, RSP
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC3, archivo_escritura) ; } // SUB RBX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xDB, archivo_escritura) ; } // SUB RBX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xCB, archivo_escritura) ; } // SUB RBX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD3, archivo_escritura) ; } // SUB RBX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF3, archivo_escritura) ; } // SUB RBX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xFB, archivo_escritura) ; } // SUB RBX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xE3, archivo_escritura) ; } // SUB RBX, RSP
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC1, archivo_escritura) ; } // SUB RCX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD9, archivo_escritura) ; } // SUB RCX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC9, archivo_escritura) ; } // SUB RCX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD1, archivo_escritura) ; } // SUB RCX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF1, archivo_escritura) ; } // SUB RCX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF9, archivo_escritura) ; } // SUB RCX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xE1, archivo_escritura) ; } // SUB RCX, RSP
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC2, archivo_escritura) ; } // SUB RDX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xDA, archivo_escritura) ; } // SUB RDX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xCA, archivo_escritura) ; } // SUB RDX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD2, archivo_escritura) ; } // SUB RDX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF2, archivo_escritura) ; } // SUB RDX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xFA, archivo_escritura) ; } // SUB RDX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xE2, archivo_escritura) ; } // SUB RDX, RSP
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC6, archivo_escritura) ; } // SUB RSI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xDE, archivo_escritura) ; } // SUB RSI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xCE, archivo_escritura) ; } // SUB RSI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD6, archivo_escritura) ; } // SUB RSI, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF6, archivo_escritura) ; } // SUB RSI, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xFE, archivo_escritura) ; } // SUB RSI, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xE6, archivo_escritura) ; } // SUB RSI, RSP
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC7, archivo_escritura) ; } // SUB RDI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xDF, archivo_escritura) ; } // SUB RDI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xCF, archivo_escritura) ; } // SUB RDI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD7, archivo_escritura) ; } // SUB RDI, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF7, archivo_escritura) ; } // SUB RDI, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xFF, archivo_escritura) ; } // SUB RDI, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xE7, archivo_escritura) ; } // SUB RDI, RSP
                }
                else if(strcmp(token_destino, "r_pila64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xC4, archivo_escritura) ; } // SUB RSP, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xDC, archivo_escritura) ; } // SUB RSP, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xCC, archivo_escritura) ; } // SUB RSP, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xD4, archivo_escritura) ; } // SUB RSP, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xF4, archivo_escritura) ; } // SUB RSP, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xFC, archivo_escritura) ; } // SUB RSP, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x29, archivo_escritura) ; fputc(0xE4, archivo_escritura) ; } // SUB RSP, RSP
                }
            } 
        }
        
        else if(strcmp(token, "comparar") == 0) {
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador32") == 0) {
                    fputc(0x3D, archivo_escritura) ;
                }
                else if(strcmp(token_registros, "r_base32") == 0) {
                    fputc(0x81, archivo_escritura) ; fputc(0xFB, archivo_escritura) ;
                }
                else if(strcmp(token_registros, "r_acumulador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x3D, archivo_escritura) ;
                    es_64 = 1 ;
                }
                else if(strcmp(token_registros, "r_base64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xFB, archivo_escritura) ;
                    es_64 = 1 ;
                }
                else if(strcmp(token_registros, "r_contador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xF9, archivo_escritura) ; // CMP RCX, imm32
                }
                else if(strcmp(token_registros, "r_datos64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xFA, archivo_escritura) ; // CMP RDX, imm32
                }
                else if(strcmp(token_registros, "r_puntero64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xFE, archivo_escritura) ; // CMP RSI, imm32
                }
                else if(strcmp(token_registros, "r_destino64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xFF, archivo_escritura) ; // CMP RDI, imm32
                }
                else if(strcmp(token_registros, "r_pila64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x81, archivo_escritura) ; fputc(0xFC, archivo_escritura) ; // CMP RSP, imm32
                }
                
                int numero = atoi(token_numero) ;
                fwrite(&numero, sizeof(int), 1, archivo_escritura) ;
            }
            else {
                // MODO REGISTRO A REGISTRO
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD8, archivo_escritura) ; } // CMP RAX, RBX
                    else if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC0, archivo_escritura) ; } // CMP RAX, RAX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC8, archivo_escritura) ; } // CMP RAX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD0, archivo_escritura) ; } // CMP RAX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF0, archivo_escritura) ; } // CMP RAX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF8, archivo_escritura) ; } // CMP RAX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xE0, archivo_escritura) ; } // CMP RAX, RSP
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC3, archivo_escritura) ; } // CMP RBX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xDB, archivo_escritura) ; } // CMP RBX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xCB, archivo_escritura) ; } // CMP RBX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD3, archivo_escritura) ; } // CMP RBX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF3, archivo_escritura) ; } // CMP RBX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xFB, archivo_escritura) ; } // CMP RBX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xE3, archivo_escritura) ; } // CMP RBX, RSP
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC1, archivo_escritura) ; } // CMP RCX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD9, archivo_escritura) ; } // CMP RCX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC9, archivo_escritura) ; } // CMP RCX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD1, archivo_escritura) ; } // CMP RCX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF1, archivo_escritura) ; } // CMP RCX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF9, archivo_escritura) ; } // CMP RCX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xE1, archivo_escritura) ; } // CMP RCX, RSP
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC2, archivo_escritura) ; } // CMP RDX, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xDA, archivo_escritura) ; } // CMP RDX, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xCA, archivo_escritura) ; } // CMP RDX, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD2, archivo_escritura) ; } // CMP RDX, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF2, archivo_escritura) ; } // CMP RDX, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xFA, archivo_escritura) ; } // CMP RDX, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xE2, archivo_escritura) ; } // CMP RDX, RSP
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC6, archivo_escritura) ; } // CMP RSI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xDE, archivo_escritura) ; } // CMP RSI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xCE, archivo_escritura) ; } // CMP RSI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD6, archivo_escritura) ; } // CMP RSI, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF6, archivo_escritura) ; } // CMP RSI, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xFE, archivo_escritura) ; } // CMP RSI, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xE6, archivo_escritura) ; } // CMP RSI, RSP
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC7, archivo_escritura) ; } // CMP RDI, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xDF, archivo_escritura) ; } // CMP RDI, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xCF, archivo_escritura) ; } // CMP RDI, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD7, archivo_escritura) ; } // CMP RDI, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF7, archivo_escritura) ; } // CMP RDI, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xFF, archivo_escritura) ; } // CMP RDI, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xE7, archivo_escritura) ; } // CMP RDI, RSP
                }
                else if(strcmp(token_destino, "r_pila64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xC4, archivo_escritura) ; } // CMP RSP, RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xDC, archivo_escritura) ; } // CMP RSP, RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xCC, archivo_escritura) ; } // CMP RSP, RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xD4, archivo_escritura) ; } // CMP RSP, RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xF4, archivo_escritura) ; } // CMP RSP, RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xFC, archivo_escritura) ; } // CMP RSP, RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x39, archivo_escritura) ; fputc(0xE4, archivo_escritura) ; } // CMP RSP, RSP
                }
            } 
        }
        else if(strcmp(token, "multiplicar") == 0) {
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO (imul reg, reg, imm32)
                if(strcmp(token_registros, "r_acumulador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x69, archivo_escritura) ; fputc(0xC0, archivo_escritura) ; // IMUL RAX, RAX, imm32
                }
                else if(strcmp(token_registros, "r_base64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x69, archivo_escritura) ; fputc(0xDB, archivo_escritura) ; // IMUL RBX, RBX, imm32
                }
                else if(strcmp(token_registros, "r_contador64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x69, archivo_escritura) ; fputc(0xC9, archivo_escritura) ; // IMUL RCX, RCX, imm32
                }
                else if(strcmp(token_registros, "r_datos64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x69, archivo_escritura) ; fputc(0xD2, archivo_escritura) ; // IMUL RDX, RDX, imm32
                }
                else if(strcmp(token_registros, "r_puntero64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x69, archivo_escritura) ; fputc(0xF6, archivo_escritura) ; // IMUL RSI, RSI, imm32
                }
                else if(strcmp(token_registros, "r_destino64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x69, archivo_escritura) ; fputc(0xFF, archivo_escritura) ; // IMUL RDI, RDI, imm32
                }
                else if(strcmp(token_registros, "r_pila64") == 0) {
                    fputc(0x48, archivo_escritura) ; fputc(0x69, archivo_escritura) ; fputc(0xE4, archivo_escritura) ; // IMUL RSP, RSP, imm32
                }
                else if(strcmp(token_registros, "r_acumulador32") == 0) {
                    fputc(0x69, archivo_escritura) ; fputc(0xC0, archivo_escritura) ; // IMUL EAX, EAX, imm32
                }
                else if(strcmp(token_registros, "r_base32") == 0) {
                    fputc(0x69, archivo_escritura) ; fputc(0xDB, archivo_escritura) ; // IMUL EBX, EBX, imm32
                }
                int numero = atoi(token_numero) ;
                fwrite(&numero, sizeof(int), 1, archivo_escritura) ;
            }
            else {
                // MODO REGISTRO A REGISTRO (imul destino, origen)
                char *token_origen = token_numero;
                char *token_destino = token_registros;
                
                if(strcmp(token_destino, "r_acumulador64") == 0) {
                    if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC3, archivo_escritura) ; } // RAX * RBX
                    else if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC0, archivo_escritura) ; } // RAX * RAX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC1, archivo_escritura) ; } // RAX * RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC2, archivo_escritura) ; } // RAX * RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC6, archivo_escritura) ; } // RAX * RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC7, archivo_escritura) ; } // RAX * RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC4, archivo_escritura) ; } // RAX * RSP
                }
                else if(strcmp(token_destino, "r_base64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD8, archivo_escritura) ; } // RBX * RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xDB, archivo_escritura) ; } // RBX * RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD9, archivo_escritura) ; } // RBX * RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xDA, archivo_escritura) ; } // RBX * RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xDE, archivo_escritura) ; } // RBX * RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xDF, archivo_escritura) ; } // RBX * RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xDC, archivo_escritura) ; } // RBX * RSP
                }
                else if(strcmp(token_destino, "r_contador64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC8, archivo_escritura) ; } // RCX * RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xCB, archivo_escritura) ; } // RCX * RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xC9, archivo_escritura) ; } // RCX * RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xCA, archivo_escritura) ; } // RCX * RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xCE, archivo_escritura) ; } // RCX * RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xCF, archivo_escritura) ; } // RCX * RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xCC, archivo_escritura) ; } // RCX * RSP
                }
                else if(strcmp(token_destino, "r_datos64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD0, archivo_escritura) ; } // RDX * RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD3, archivo_escritura) ; } // RDX * RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD1, archivo_escritura) ; } // RDX * RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD2, archivo_escritura) ; } // RDX * RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD6, archivo_escritura) ; } // RDX * RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD7, archivo_escritura) ; } // RDX * RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xD4, archivo_escritura) ; } // RDX * RSP
                }
                else if(strcmp(token_destino, "r_puntero64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF0, archivo_escritura) ; } // RSI * RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF3, archivo_escritura) ; } // RSI * RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF1, archivo_escritura) ; } // RSI * RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF2, archivo_escritura) ; } // RSI * RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF6, archivo_escritura) ; } // RSI * RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF7, archivo_escritura) ; } // RSI * RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF4, archivo_escritura) ; } // RSI * RSP
                }
                else if(strcmp(token_destino, "r_destino64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF8, archivo_escritura) ; } // RDI * RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xFB, archivo_escritura) ; } // RDI * RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xF9, archivo_escritura) ; } // RDI * RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xFA, archivo_escritura) ; } // RDI * RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xFE, archivo_escritura) ; } // RDI * RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xFF, archivo_escritura) ; } // RDI * RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xFC, archivo_escritura) ; } // RDI * RSP
                }
                else if(strcmp(token_destino, "r_pila64") == 0) {
                    if(strcmp(token_origen, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xE0, archivo_escritura) ; } // RSP * RAX
                    else if(strcmp(token_origen, "r_base64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xE3, archivo_escritura) ; } // RSP * RBX
                    else if(strcmp(token_origen, "r_contador64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xE1, archivo_escritura) ; } // RSP * RCX
                    else if(strcmp(token_origen, "r_datos64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xE2, archivo_escritura) ; } // RSP * RDX
                    else if(strcmp(token_origen, "r_puntero64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xE6, archivo_escritura) ; } // RSP * RSI
                    else if(strcmp(token_origen, "r_destino64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xE7, archivo_escritura) ; } // RSP * RDI
                    else if(strcmp(token_origen, "r_pila64") == 0) { fputc(0x48, archivo_escritura) ; fputc(0x0F, archivo_escritura) ; fputc(0xAF, archivo_escritura) ; fputc(0xE4, archivo_escritura) ; } // RSP * RSP
                }
            }
        }
        else if(strcmp(token, "dividir") == 0) {
            // Sintaxis: dividir <registro_divisor>
            // El dividendo siempre debe estar en r_acumulador64 (RAX)
            
            if(strcmp(token_numero, "r_acumulador64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV RAX
            }
            else if(strcmp(token_numero, "r_base64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV RBX
            }
            else if(strcmp(token_numero, "r_contador64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV RCX
            }
            else if(strcmp(token_numero, "r_datos64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV RDX
            }
            else if(strcmp(token_numero, "r_puntero64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV RSI
            }
            else if(strcmp(token_numero, "r_destino64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV RDI
            }
            else if(strcmp(token_numero, "r_pila64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV RSP
            }
        }
    } // <--- CIERRA EL WHILE

    // Calcular los tamaños reales
    long posicion_final = ftell(archivo_escritura);
    
    // Si el usuario no puso .codigo explícito, asumimos que empieza tras la cabecera
    if (offset_codigo == -1) offset_codigo = 80; 
    
    long tamano_codigo = 0;
    long tamano_datos = 0;

    // Averiguar qué sección va primero y calcular tamaños
    if (offset_datos != -1 && offset_datos < offset_codigo) {
        // Los datos van primero, luego el código
        tamano_datos = offset_codigo - offset_datos;
        tamano_codigo = posicion_final - offset_codigo;
    } else if (offset_datos != -1 && offset_datos > offset_codigo) {
        // El código va primero, luego los datos
        tamano_codigo = offset_datos - offset_codigo;
        tamano_datos = posicion_final - offset_datos;
    } else {
        // Solo hay código (o está vacío)
        tamano_codigo = posicion_final - offset_codigo;
        tamano_datos = 0;
        offset_datos = 0; // El cargador ignorará los datos si el offset es 0
    }

    // Volver al principio y rellenar la cabecera JMA
    fseek(archivo_escritura, 0, SEEK_SET);
    
    // Firma mágica: "JMA\0" (0x4A 0x4D 0x41 0x00)
    fputc(0x4A, archivo_escritura); fputc(0x4D, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x00, archivo_escritura);
    
    // Versión: 1.0 (0x01 0x00 0x00 0x00)
    fputc(0x01, archivo_escritura); fputc(0x00, archivo_escritura); fputc(0x00, archivo_escritura); fputc(0x00, archivo_escritura);
    
    // Punto de entrada (0x400000)
    long long entry_point = 0x400000;
    fwrite(&entry_point, sizeof(long long), 1, archivo_escritura);
    
    // Offset del código
    fwrite(&offset_codigo, sizeof(long), 1, archivo_escritura);
    
    // Tamaño del código
    fwrite(&tamano_codigo, sizeof(long), 1, archivo_escritura);
    
    // Offset de los datos
    fwrite(&offset_datos, sizeof(long), 1, archivo_escritura);
    
    // Tamaño de los datos
    fwrite(&tamano_datos, sizeof(long), 1, archivo_escritura);
    
    // Relleno hasta 80 bytes (80 - 48 = 32)
    for (int i = 0; i < 32; i++) {
        fputc(0x00, archivo_escritura);
    }
} // <--- CIERRA LA FUNCIÓN

// CARGADOR JMA (Integrado)
typedef struct {
    char firma[4];
    int version;
    long long entry_point;
    long offset_codigo;
    long tamano_codigo;
    long offset_datos;
    long tamano_datos;
    char relleno[32];
} CabeceraJMA;

void cargar_y_ejecutar(const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "rb");
    if (archivo == NULL) {
        printf("Error: No se pudo abrir %s\n", nombre_archivo);
        return;
    }

    CabeceraJMA cabecera;
    fread(&cabecera, sizeof(CabeceraJMA), 1, archivo);

    if (memcmp(cabecera.firma, "JMA", 3) != 0) {
        printf("Error: El archivo no es un ejecutable JMA valido.\n");
        fclose(archivo);
        return;
    }

    // 1. Pedir memoria para TODO el archivo (cabecera + código + datos)
    long tamano_total = cabecera.offset_datos + cabecera.tamano_datos;
    if (tamano_total < cabecera.offset_codigo + cabecera.tamano_codigo) {
        tamano_total = cabecera.offset_codigo + cabecera.tamano_codigo;
    }
    
    void *memoria = mmap((void*)0x400000, tamano_total, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (memoria == MAP_FAILED) {
        printf("Error en mmap\n");
        fclose(archivo);
        return;
    }

    // 2. Copiar el código en su offset original
    fseek(archivo, cabecera.offset_codigo, SEEK_SET);
    fread((char*)memoria + cabecera.offset_codigo, 1, cabecera.tamano_codigo, archivo);

    // 3. Copiar los datos en su offset original
    if (cabecera.tamano_datos > 0) {
        fseek(archivo, cabecera.offset_datos, SEEK_SET);
        fread((char*)memoria + cabecera.offset_datos, 1, cabecera.tamano_datos, archivo);
    }
    
    fclose(archivo);

    printf("Ejecutando %s (JMA v%d)...\n", nombre_archivo, cabecera.version);

    // 4. Saltar al código (que está en memoria + offset_codigo)
    void *punto_entrada = (char*)memoria + cabecera.offset_codigo;
    
    asm volatile (
        "andq $-16, %%rsp\n"
        "jmp *%0\n"
        : 
        : "r" (punto_entrada)
        : "memory"
    );
}