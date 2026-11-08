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


int obtener_tamano_registro(const char *nombre) {
    if (strstr(nombre, "64") != NULL) return 64;
    if (strstr(nombre, "32") != NULL) return 32;
    if (strstr(nombre, "16") != NULL) return 16;
    if (strstr(nombre, "8") != NULL) return 8;
    return 0; // Error
}
// Convierte el nombre del registro en un número (0 a 15)
int obtener_id_registro(const char *nombre) {
    // Registros de 64 bits (clásicos)
    if (strcmp(nombre, "r_acumulador64") == 0) return 0; // RAX
    if (strcmp(nombre, "r_contador64") == 0) return 1;   // RCX
    if (strcmp(nombre, "r_datos64") == 0) return 2;      // RDX
    if (strcmp(nombre, "r_base64") == 0) return 3;       // RBX
    if (strcmp(nombre, "r_pila64") == 0) return 4;       // RSP
    if (strcmp(nombre, "r_puntero64") == 0) return 6;    // RSI  <-- AÑADIDO
    if (strcmp(nombre, "r_destino64") == 0) return 7;    // RDI

    // Registros de 64 bits (especiales)
    if (strcmp(nombre, "r8_64") == 0) return 8;
    if (strcmp(nombre, "r9_64") == 0) return 9;
    if (strcmp(nombre, "r10_64") == 0) return 10;
    if (strcmp(nombre, "r11_64") == 0) return 11;
    if (strcmp(nombre, "r12_64") == 0) return 12;
    if (strcmp(nombre, "r13_64") == 0) return 13;
    if (strcmp(nombre, "r14_64") == 0) return 14;
    if (strcmp(nombre, "r15_64") == 0) return 15;

    // Registros de 32 bits (clásicos)
    if (strcmp(nombre, "r_acumulador32") == 0) return 0; // EAX
    if (strcmp(nombre, "r_contador32") == 0) return 1;   // ECX
    if (strcmp(nombre, "r_datos32") == 0) return 2;      // EDX
    if (strcmp(nombre, "r_base32") == 0) return 3;       // EBX
    if (strcmp(nombre, "r_pila32") == 0) return 4;       // ESP  <-- AÑADIDO
    if (strcmp(nombre, "r_puntero32") == 0) return 6;    // ESI
    if (strcmp(nombre, "r_destino32") == 0) return 7;    // EDI

    // Registros de 32 bits (especiales)
    if (strcmp(nombre, "r8_32") == 0) return 8;
    if (strcmp(nombre, "r9_32") == 0) return 9;
    if (strcmp(nombre, "r10_32") == 0) return 10;
    if (strcmp(nombre, "r11_32") == 0) return 11;
    if (strcmp(nombre, "r12_32") == 0) return 12;
    if (strcmp(nombre, "r13_32") == 0) return 13;
    if (strcmp(nombre, "r14_32") == 0) return 14;
    if (strcmp(nombre, "r15_32") == 0) return 15;

    // Registros de 16 bits (clásicos)
    if (strcmp(nombre, "r_acumulador16") == 0) return 0; // AX
    if (strcmp(nombre, "r_contador16") == 0) return 1;   // CX
    if (strcmp(nombre, "r_datos16") == 0) return 2;      // DX
    if (strcmp(nombre, "r_base16") == 0) return 3;       // BX
    if (strcmp(nombre, "r_pila16") == 0) return 4;       // SP
    if (strcmp(nombre, "r_puntero16") == 0) return 6;    // SI
    if (strcmp(nombre, "r_destino16") == 0) return 7;    // DI

    // Registros de 16 bits (especiales)
    if (strcmp(nombre, "r8_16") == 0) return 8;
    if (strcmp(nombre, "r9_16") == 0) return 9;
    if (strcmp(nombre, "r10_16") == 0) return 10;
    if (strcmp(nombre, "r11_16") == 0) return 11;
    if (strcmp(nombre, "r12_16") == 0) return 12;
    if (strcmp(nombre, "r13_16") == 0) return 13;
    if (strcmp(nombre, "r14_16") == 0) return 14;
    if (strcmp(nombre, "r15_16") == 0) return 15;
    
    // Registros de 8 bits (clásicos)
    if (strcmp(nombre, "r_acumulador8") == 0) return 0; // AL
    if (strcmp(nombre, "r_contador8") == 0) return 1;   // CL
    if (strcmp(nombre, "r_datos8") == 0) return 2;      // DL
    if (strcmp(nombre, "r_base8") == 0) return 3;       // BL
    if (strcmp(nombre, "r_pila8") == 0) return 4;       // SPL
    if (strcmp(nombre, "r_puntero8") == 0) return 6;    // SIL
    if (strcmp(nombre, "r_destino8") == 0) return 7;    // DIL

    // Registros de 8 bits (especiales)
    if (strcmp(nombre, "r8_8") == 0) return 8;
    if (strcmp(nombre, "r9_8") == 0) return 9;
    if (strcmp(nombre, "r10_8") == 0) return 10;
    if (strcmp(nombre, "r11_8") == 0) return 11;
    if (strcmp(nombre, "r12_8") == 0) return 12;
    if (strcmp(nombre, "r13_8") == 0) return 13;
    if (strcmp(nombre, "r14_8") == 0) return 14;
    if (strcmp(nombre, "r15_8") == 0) return 15;

    // Si no lo encuentra, devuelve -1 (error)
    return -1; 
}

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
            int id_reg = obtener_id_registro(token_numero);
            int tam_reg = obtener_tamano_registro(token_numero);

            // PUSH no soporta registros de 8 bits (los rechazamos silenciosamente)
            if (id_reg != -1 && tam_reg != 0 && tam_reg != 8) {
                int rex = 0x40;
                if (tam_reg == 64) rex |= 0x08; // REX.W
                if (id_reg >= 8) rex |= 0x01;   // REX.B

                if (tam_reg == 16) fputc(0x66, archivo_escritura); // Prefijo de 16 bits
                if (rex != 0x40) fputc(rex, archivo_escritura);    // Prefijo REX

                fputc(0x50 + (id_reg & 7), archivo_escritura);     // Opcode PUSH
            }
        }
        
        else if(strcmp(token, "sacar") == 0) {
            int id_reg = obtener_id_registro(token_numero);
            int tam_reg = obtener_tamano_registro(token_numero);

            // POP no soporta registros de 8 bits
            if (id_reg != -1 && tam_reg != 0 && tam_reg != 8) {
                int rex = 0x40;
                if (tam_reg == 64) rex |= 0x08; // REX.W
                if (id_reg >= 8) rex |= 0x01;   // REX.B

                if (tam_reg == 16) fputc(0x66, archivo_escritura); // Prefijo de 16 bits
                if (rex != 0x40) fputc(rex, archivo_escritura);    // Prefijo REX

                fputc(0x58 + (id_reg & 7), archivo_escritura);     // Opcode POP
            }
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
                // --- LÓGICA DE MEMORIA INDIRECTA MATEMÁTICA ---
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

                int id_destino = obtener_id_registro(token_registros);
                int id_base = obtener_id_registro(base_reg);
                int tam_destino = obtener_tamano_registro(token_registros);

                if (id_destino != -1 && id_base != -1) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08; // REX.W
                    if (id_destino >= 8) rex |= 0x04;    // REX.R
                    if (id_base >= 8) rex |= 0x01;       // REX.B

                    if (tam_destino == 16) fputc(0x66, archivo_escritura);
                    if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura);

                    // Opcode: 0x8A para 8 bits, 0x8B para el resto
                    int opcode = (tam_destino == 8) ? 0x8A : 0x8B;
                    fputc(opcode, archivo_escritura);

                    // Cálculo del ModR/M y SIB
                    int mod = 0;
                    if (offset != 0) mod = 0x80; // Usar desplazamiento de 32 bits
                    else if ((id_base & 7) == 5) mod = 0x40; // RBP o R13 sin offset necesitan disp8=0

                    int rm = id_base & 7;
                    int reg = id_destino & 7;

                    if (rm == 4) { // RSP o R12 requieren byte SIB
                        fputc(mod | (reg << 3) | 4, archivo_escritura);
                        fputc(0x24, archivo_escritura); // SIB: Scale=0, Index=4 (none), Base=4 (RSP)
                    } else {
                        fputc(mod | (reg << 3) | rm, archivo_escritura);
                    }

                    if (mod == 0x80) {
                        fwrite(&offset, sizeof(int), 1, archivo_escritura);
                    } else if (mod == 0x40) {
                        fputc(0x00, archivo_escritura); // disp8 = 0
                    }
                }
            }
             else {
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

            int id_origen = obtener_id_registro(token_numero);
            int id_base = obtener_id_registro(base_reg);
            int tam_origen = obtener_tamano_registro(token_numero);

            if (id_origen != -1 && id_base != -1) {
                int rex = 0x40;
                if (tam_origen == 64) rex |= 0x08; // REX.W
                if (id_origen >= 8) rex |= 0x04;   // REX.R
                if (id_base >= 8) rex |= 0x01;     // REX.B

                if (tam_origen == 16) fputc(0x66, archivo_escritura);
                if (rex != 0x40 || tam_origen == 8) fputc(rex, archivo_escritura);

                // Opcode: 0x88 para 8 bits, 0x89 para el resto
                int opcode = (tam_origen == 8) ? 0x88 : 0x89;
                fputc(opcode, archivo_escritura);

                // Cálculo del ModR/M y SIB
                int mod = 0;
                if (offset != 0) mod = 0x80; // Usar desplazamiento de 32 bits
                else if ((id_base & 7) == 5) mod = 0x40; // RBP o R13 sin offset necesitan disp8=0

                int rm = id_base & 7;
                int reg = id_origen & 7;

                if (rm == 4) { // RSP o R12 requieren byte SIB
                    fputc(mod | (reg << 3) | 4, archivo_escritura);
                    fputc(0x24, archivo_escritura); // SIB: Scale=0, Index=4 (none), Base=4 (RSP)
                } else {
                    fputc(mod | (reg << 3) | rm, archivo_escritura);
                }

                if (mod == 0x80) {
                    fwrite(&offset, sizeof(int), 1, archivo_escritura);
                } else if (mod == 0x40) {
                    fputc(0x00, archivo_escritura); // disp8 = 0
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

            int id_origen = obtener_id_registro(token_numero);
            int id_base = obtener_id_registro(base_reg);

            if (id_origen != -1 && id_base != -1) {
                int rex = 0x40;
                if (id_origen >= 8) rex |= 0x04;   // REX.R
                if (id_base >= 8) rex |= 0x01;     // REX.B

                if (rex != 0x40) fputc(rex, archivo_escritura);

                fputc(0x88, archivo_escritura); // Opcode: MOV r/m8, r8

                int mod = 0;
                if (offset != 0) mod = 0x80; 
                else if ((id_base & 7) == 5) mod = 0x40; 

                int rm = id_base & 7;
                int reg = id_origen & 7;

                if (rm == 4) { 
                    fputc(mod | (reg << 3) | 4, archivo_escritura);
                    fputc(0x24, archivo_escritura); 
                } else {
                    fputc(mod | (reg << 3) | rm, archivo_escritura);
                }

                if (mod == 0x80) fwrite(&offset, sizeof(int), 1, archivo_escritura);
                else if (mod == 0x40) fputc(0x00, archivo_escritura);
            }
        }
                else if(strcmp(token, "guardar_doble_puntero") == 0) {
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

            int id_origen = obtener_id_registro(token_numero);
            int id_base = obtener_id_registro(base_reg);

            if (id_origen != -1 && id_base != -1) {
                int rex = 0x40;
                if (id_origen >= 8) rex |= 0x04;   // REX.R
                if (id_base >= 8) rex |= 0x01;     // REX.B

                if (rex != 0x40) fputc(rex, archivo_escritura);

                fputc(0x89, archivo_escritura); // Opcode: MOV r/m32, r32

                int mod = 0;
                if (offset != 0) mod = 0x80; 
                else if ((id_base & 7) == 5) mod = 0x40; 

                int rm = id_base & 7;
                int reg = id_origen & 7;

                if (rm == 4) { 
                    fputc(mod | (reg << 3) | 4, archivo_escritura);
                    fputc(0x24, archivo_escritura); 
                } else {
                    fputc(mod | (reg << 3) | rm, archivo_escritura);
                }

                if (mod == 0x80) fwrite(&offset, sizeof(int), 1, archivo_escritura);
                else if (mod == 0x40) fputc(0x00, archivo_escritura);
            }
        }
        else if(strcmp(token, "guardar_indirecto") == 0) {
            char reg_dir[50];
            strcpy(reg_dir, token_registros);
            if (reg_dir[0] == '[') {
                memmove(reg_dir, reg_dir + 1, strlen(reg_dir));
                reg_dir[strlen(reg_dir) - 1] = '\0';
            }

            int id_origen = obtener_id_registro(token_numero);
            int id_base = obtener_id_registro(reg_dir);

            if (id_origen != -1 && id_base != -1) {
                int rex = 0x48; // REX.W activado por defecto (64 bits)
                if (id_origen >= 8) rex |= 0x04;   // REX.R
                if (id_base >= 8) rex |= 0x01;     // REX.B

                fputc(rex, archivo_escritura);
                fputc(0x89, archivo_escritura); // Opcode: MOV r/m64, r64

                int mod = 0;
                if ((id_base & 7) == 5) mod = 0x40; // RBP o R13 sin offset necesitan disp8=0

                int rm = id_base & 7;
                int reg = id_origen & 7;

                if (rm == 4) { 
                    fputc(mod | (reg << 3) | 4, archivo_escritura);
                    fputc(0x24, archivo_escritura); 
                } else {
                    fputc(mod | (reg << 3) | rm, archivo_escritura);
                }

                if (mod == 0x40) fputc(0x00, archivo_escritura);
            }
        }
        else if(strcmp(token, "desplazar_derecha") == 0) {
            if (isdigit(token_numero[0])) {
                // MODO INMEDIATO (Manual y auditable)
                int num = atoi(token_numero);

                // 8 BITS (C0 /5 ib)
                if(strcmp(token_registros, "r_acumulador8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila8") == 0) { fputc(0x40, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero8") == 0) { fputc(0x40, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino8") == 0) { fputc(0x40, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // 16 BITS (66 C1 /5 ib)
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // 32 BITS (C1 /5 ib)
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xEF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // 64 BITS (48 C1 /5 ib)
                else if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xEF, archivo_escritura); }

                fputc(num, archivo_escritura);
            }
            else {
                // MODO REGISTRO A REGISTRO (Atajo Matemático - Solo CL)
                if (strcmp(token_numero, "r_contador8") == 0) {
                    int id_destino = obtener_id_registro(token_registros);
                    int tam_destino = obtener_tamano_registro(token_registros);
                    if (id_destino != -1 && tam_destino != 0) {
                        int rex = 0x40;
                        if (tam_destino == 64) rex |= 0x08;
                        if (id_destino >= 8) rex |= 0x01;

                        if (tam_destino == 16) fputc(0x66, archivo_escritura);
                        if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura);

                        int opcode = (tam_destino == 8) ? 0xD2 : 0xD3;
                        fputc(opcode, archivo_escritura);

                        int modrm = 0xC0 | (5 << 3) | (id_destino & 7); // /5 para SHR
                        fputc(modrm, archivo_escritura);
                    }
                }
            }
        }
        else if(strcmp(token, "desplazar_izquierda") == 0) {
            if (isdigit(token_numero[0])) {
                // MODO INMEDIATO (Manual y auditable)
                int num = atoi(token_numero);

                // 8 BITS (C0 /4 ib)
                if(strcmp(token_registros, "r_acumulador8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base8") == 0) { fputc(0xC0, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila8") == 0) { fputc(0x40, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero8") == 0) { fputc(0x40, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino8") == 0) { fputc(0x40, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_8") == 0) { fputc(0x41, archivo_escritura); fputc(0xC0, archivo_escritura); fputc(0xE7, archivo_escritura); }

                // 16 BITS (66 C1 /4 ib)
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE7, archivo_escritura); }

                // 32 BITS (C1 /4 ib)
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0xC1, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x41, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE7, archivo_escritura); }

                // 64 BITS (48 C1 /4 ib)
                else if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_64") == 0) { fputc(0x49, archivo_escritura); fputc(0xC1, archivo_escritura); fputc(0xE7, archivo_escritura); }

                fputc(num, archivo_escritura);
            }
            else {
                // MODO REGISTRO A REGISTRO (Atajo Matemático - Solo CL)
                if (strcmp(token_numero, "r_contador8") == 0) {
                    int id_destino = obtener_id_registro(token_registros);
                    int tam_destino = obtener_tamano_registro(token_registros);
                    if (id_destino != -1 && tam_destino != 0) {
                        int rex = 0x40;
                        if (tam_destino == 64) rex |= 0x08;
                        if (id_destino >= 8) rex |= 0x01;

                        if (tam_destino == 16) fputc(0x66, archivo_escritura);
                        if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura);

                        int opcode = (tam_destino == 8) ? 0xD2 : 0xD3;
                        fputc(opcode, archivo_escritura);

                        int modrm = 0xC0 | (4 << 3) | (id_destino & 7); // /4 para SHL
                        fputc(modrm, archivo_escritura);
                    }
                }
            }
        }
        else if(strcmp(token, "y") == 0) {
            
            // --- ATAJO MATEMÁTICO (REGISTRO A REGISTRO) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                if (id_origen != -1 && id_destino != -1 && tam_origen == tam_destino) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08; // REX.W
                    if (id_origen >= 8) rex |= 0x04;    // REX.R (origen)
                    if (id_destino >= 8) rex |= 0x01;   // REX.B (destino)

                    int opcode = (tam_destino == 8) ? 0x20 : 0x21;
                    int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);

                    if (tam_destino == 16) fputc(0x66, archivo_escritura); 
                    if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura); 
                    
                    fputc(opcode, archivo_escritura);
                    fputc(modrm, archivo_escritura);
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---
            
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                if(strcmp(token_registros, "r_acumulador8") == 0) { fputc(0x24, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador8") == 0) { fputc(0x80, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos8") == 0) { fputc(0x80, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base8") == 0) { fputc(0x80, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE7, archivo_escritura); }

                // 16 BITS (66 81 /4 iw)
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x25, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE7, archivo_escritura); }

                // 32 BITS (81 /4 id)
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0x25, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0x81, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0x81, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0x81, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0x81, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0x81, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0x81, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE7, archivo_escritura); }

                // 64 BITS (48 81 /4 id)
                else if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x25, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE7, archivo_escritura); }

                // Escribir el inmediato con el tamaño correcto
                int tam = obtener_tamano_registro(token_registros);
                int numero = atoi(token_numero);
                if (tam == 8) { fputc(numero, archivo_escritura); }
                else if (tam == 16) { short n = (short)numero; fwrite(&n, sizeof(short), 1, archivo_escritura); }
                else { fwrite(&numero, sizeof(int), 1, archivo_escritura); }
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
            // --- ATAJO MATEMÁTICO (REGISTRO A REGISTRO) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                if (id_origen != -1 && id_destino != -1 && tam_origen == tam_destino) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08;
                    if (id_origen >= 8) rex |= 0x04;
                    if (id_destino >= 8) rex |= 0x01;

                    int opcode = (tam_destino == 8) ? 0x08 : 0x09;
                    int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);

                    if (tam_destino == 16) fputc(0x66, archivo_escritura); 
                    if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura); 
                    
                    fputc(opcode, archivo_escritura);
                    fputc(modrm, archivo_escritura);
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---


            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                // 8 BITS (80 /1 ib)
                if(strcmp(token_registros, "r_acumulador8") == 0) { fputc(0x0C, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador8") == 0) { fputc(0x80, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos8") == 0) { fputc(0x80, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base8") == 0) { fputc(0x80, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCD, archivo_escritura); }
                else if(strcmp(token_registros, "r14_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xCF, archivo_escritura); }

                // 16 BITS (66 81 /1 iw)
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x0D, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCD, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCF, archivo_escritura); }

                // 32 BITS (81 /1 id)
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0x0D, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0x81, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0x81, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0x81, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0x81, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0x81, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0x81, archivo_escritura); fputc(0xCF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCD, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCF, archivo_escritura); }

                // 64 BITS (48 81 /1 id)
                else if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x0D, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCF, archivo_escritura); }
                else if(strcmp(token_registros, "r8_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCD, archivo_escritura); }
                else if(strcmp(token_registros, "r14_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xCF, archivo_escritura); }

                int tam = obtener_tamano_registro(token_registros);
                int numero = atoi(token_numero);
                if (tam == 8) { fputc(numero, archivo_escritura); }
                else if (tam == 16) { short n = (short)numero; fwrite(&n, sizeof(short), 1, archivo_escritura); }
                else { fwrite(&numero, sizeof(int), 1, archivo_escritura); }
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
            
            // --- ATAJO MATEMÁTICO PARA REGISTROS NUEVOS (SUMAR) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                // Solo actuamos si ambos registros son válidos y del mismo tamaño
                if (id_origen != -1 && id_destino != -1 && tam_origen == tam_destino) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08; // REX.W
                    if (id_origen >= 8) rex |= 0x04; // REX.R
                    if (id_destino >= 8) rex |= 0x01; // REX.B

                    // ModR/M: Reg = Origen, R/M = Destino
                    int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);
                    
                    // Opcode: 0x00 para 8 bits, 0x01 para 16/32/64 bits
                    int opcode = (tam_destino == 8) ? 0x00 : 0x01;

                    // Prefijo de 16 bits
                    if (tam_destino == 16) fputc(0x66, archivo_escritura); 
                    
                    // Prefijo REX (obligatorio si es de 8 bits, aunque sean clásicos como SPL)
                    if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura); 
                    
                    fputc(opcode, archivo_escritura);
                    fputc(modrm, archivo_escritura);
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---
            
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                // Registros clasicos de 8 bits
                if(strcmp(token_registros, "r_acumulador8") == 0) { 
                    fputc(0x04, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_base8") == 0) { 
                    fputc(0x80, archivo_escritura); fputc(0xC3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_contador8") == 0) { 
                    fputc(0x80, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_datos8") == 0) { 
                    fputc(0x80, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_pila8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_puntero8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_destino8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC7, archivo_escritura); 
                }

                // Registros especiales de 8 bits
                else if(strcmp(token_registros, "r8_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC5, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xC7, archivo_escritura); 
                }

                // --- REGISTROS CLÁSICOS DE 16 BITS ---
                else if(strcmp(token_registros, "r_acumulador16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x05, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_base16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_contador16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_datos16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_pila16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_puntero16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_destino16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC7, archivo_escritura); 
                }
                // Registros especiales de 16 bits
                else if(strcmp(token_registros, "r8_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC5, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC7, archivo_escritura); 
                }
                // Registros de 32 bits clasicos
                if(strcmp(token_registros, "r_acumulador32") == 0) {
                    fputc(0x05, archivo_escritura) ;
                }
                else if(strcmp(token_registros, "r_base32") == 0) {
                    fputc(0x81, archivo_escritura) ;
                    fputc(0xC3, archivo_escritura) ;
                }
                else if(strcmp(token_registros, "r_contador32") == 0) { 
                    fputc(0x81, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_datos32") == 0) { 
                    fputc(0x81, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_pila32") == 0) { 
                    fputc(0x81, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_puntero32") == 0) { 
                    fputc(0x81, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_destino32") == 0) { 
                    fputc(0x81, archivo_escritura); fputc(0xC7, archivo_escritura); 
                }
                // --- REGISTROS ESPECIALES DE 32 BITS (INMEDIATO SUMAR) ---
                else if(strcmp(token_registros, "r8_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC5, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC7, archivo_escritura); 
                }
                // Registros de 64 bits clasicos
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
                // Registros de 64 bits especiales
                else if(strcmp(token_registros, "r8_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC5, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xC7, archivo_escritura); 
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
            // --- ATAJO MATEMÁTICO (REGISTRO A REGISTRO) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                if (id_origen != -1 && id_destino != -1 && tam_origen == tam_destino) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08;
                    if (id_origen >= 8) rex |= 0x04;
                    if (id_destino >= 8) rex |= 0x01;

                    int opcode = (tam_destino == 8) ? 0x30 : 0x31;
                    int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);

                    if (tam_destino == 16) fputc(0x66, archivo_escritura); 
                    if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura); 
                    
                    fputc(opcode, archivo_escritura);
                    fputc(modrm, archivo_escritura);
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---

            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO (Añadido)
                // 8 BITS (80 /6 ib)
                if(strcmp(token_registros, "r_acumulador8") == 0) { fputc(0x34, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador8") == 0) { fputc(0x80, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos8") == 0) { fputc(0x80, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base8") == 0) { fputc(0x80, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF7, archivo_escritura); }

                // 16 BITS (66 81 /6 iw)
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x35, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF7, archivo_escritura); }

                // 32 BITS (81 /6 id)
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0x35, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0x81, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0x81, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0x81, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0x81, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0x81, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0x81, archivo_escritura); fputc(0xF7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF7, archivo_escritura); }

                // 64 BITS (48 81 /6 id)
                else if(strcmp(token_registros, "r_acumulador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x35, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r_base64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino64") == 0) { fputc(0x48, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF7, archivo_escritura); }
                else if(strcmp(token_registros, "r8_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF1, archivo_escritura); }
                else if(strcmp(token_registros, "r10_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF3, archivo_escritura); }
                else if(strcmp(token_registros, "r12_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF5, archivo_escritura); }
                else if(strcmp(token_registros, "r14_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_64") == 0) { fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF7, archivo_escritura); }

                // Escribir el inmediato
                int tam = obtener_tamano_registro(token_registros);
                int numero = atoi(token_numero);
                if (tam == 8) { fputc(numero, archivo_escritura); }
                else if (tam == 16) { short n = (short)numero; fwrite(&n, sizeof(short), 1, archivo_escritura); }
                else { fwrite(&numero, sizeof(int), 1, archivo_escritura); }
            }
        }
        

        else if(strcmp(token, "copiar") == 0) {
            
                        // --- ATAJO MATEMÁTICO PARA REGISTROS NUEVOS (CON MEZCLA DE TAMAÑOS) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                if (id_origen != -1 && id_destino != -1 && tam_origen != 0 && tam_destino != 0) {
                    // Si los tamaños son iguales, MOV normal
                    if (tam_origen == tam_destino) {
                        int rex = 0x40;
                        if (tam_destino == 64) rex |= 0x08; // REX.W
                        if (id_origen >= 8) rex |= 0x04;
                        if (id_destino >= 8) rex |= 0x01;

                        int opcode = (tam_destino == 8) ? 0x88 : 0x89; // 0x88 para bytes
                        int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);

                        if (tam_destino == 16) fputc(0x66, archivo_escritura); // Prefijo 16 bits
                        if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura);
                        fputc(opcode, archivo_escritura);
                        fputc(modrm, archivo_escritura);
                    }
                    // Si el destino es MÁS GRANDE, usamos MOVZX (extensión con ceros)
                                        // Si el destino es MÁS GRANDE, usamos MOVZX (extensión con ceros)
                    else if (tam_destino > tam_origen) {
                        // Si el origen es de 32 bits, usamos MOV normal
                        if (tam_origen == 32) {
                            int opcode = 0x89;
                            int rex_mov = 0x40; // Sin REX.W porque el origen es 32 bits
                            if (id_origen >= 8) rex_mov |= 0x04; // REX.R para el origen (Reg)
                            if (id_destino >= 8) rex_mov |= 0x01; // REX.B para el destino (R/M)
                            
                            // Para MOV, Reg = Origen, R/M = Destino
                            int modrm_mov = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);

                            if (rex_mov != 0x40) fputc(rex_mov, archivo_escritura);
                            fputc(opcode, archivo_escritura);
                            fputc(modrm_mov, archivo_escritura);
                        } 
                        else {
                            // Para 8 y 16 bits usamos MOVZX
                            int rex = 0x40;
                            if (tam_destino == 64) rex |= 0x08; // REX.W
                            if (id_destino >= 8) rex |= 0x04; // REX.R para el destino (Reg)
                            if (id_origen >= 8) rex |= 0x01; // REX.B para el origen (R/M)
                            
                            // Para MOVZX, Reg = Destino, R/M = Origen
                            int modrm_movzx = 0xC0 | ((id_destino & 7) << 3) | (id_origen & 7);
                            int opcode2 = (tam_origen == 8) ? 0xB6 : 0xB7;

                            if (rex != 0x40) fputc(rex, archivo_escritura);
                            fputc(0x0F, archivo_escritura);
                            fputc(opcode2, archivo_escritura);
                            fputc(modrm_movzx, archivo_escritura);
                        }
                    }
                    // Si el destino es MÁS PEQUEÑO, el hardware trunca automáticamente (como 32->64)
                    else {
                        int rex = 0x40;
                        if (tam_destino == 64) rex |= 0x08;
                        if (id_origen >= 8) rex |= 0x04;
                        if (id_destino >= 8) rex |= 0x01;

                        int opcode = (tam_destino == 8) ? 0x88 : 0x89;
                        int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);

                        if (tam_destino == 16) fputc(0x66, archivo_escritura);
                        if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura);
                        fputc(opcode, archivo_escritura);
                        fputc(modrm, archivo_escritura);
                    }
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---

            if (isdigit(token_numero[0])) { 
                // MODO INMEDIATO
                
                // --- REGISTROS CLÁSICOS DE 8 BITS ---
                if(strcmp(token_registros, "r_acumulador8") == 0) { 
                    fputc(0xB0, archivo_escritura); 
                } // AL
                else if(strcmp(token_registros, "r_base8") == 0) { 
                    fputc(0xB3, archivo_escritura); 
                }      // BL
                else if(strcmp(token_registros, "r_contador8") == 0) { 
                    fputc(0xB1, archivo_escritura); 
                }  // CL
                else if(strcmp(token_registros, "r_datos8") == 0) { 
                    fputc(0xB2, archivo_escritura); 
                }     // DL
                else if(strcmp(token_registros, "r_pila8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0xB4, archivo_escritura); 
                }    // SPL
                else if(strcmp(token_registros, "r_puntero8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0xB6, archivo_escritura); 
                } // SIL
                else if(strcmp(token_registros, "r_destino8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0xB7, archivo_escritura); 
                } // DIL

                // --- REGISTROS ESPECIALES DE 8 BITS ---
                else if(strcmp(token_registros, "r8_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB5, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB7, archivo_escritura); 
                }

                // --- REGISTROS CLÁSICOS DE 16 BITS ---
                else if(strcmp(token_registros, "r_acumulador16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0xB8, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_base16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0xBB, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_contador16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0xB9, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_datos16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0xBA, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_pila16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0xBC, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_puntero16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0xBE, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_destino16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0xBF, archivo_escritura); 
                }
                
                // --- REGISTROS ESPECIALES DE 16 BITS ---
                else if(strcmp(token_registros, "r8_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xB8, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xB9, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xBA, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xBB, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xBC, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xBD, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xBE, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_16") == 0) { 
                    fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xBF, archivo_escritura); 
                }

                // Registros clasicos de 32 bits
                if(strcmp(token_registros, "r_acumulador32") == 0) {
                    fputc(0xB8, archivo_escritura) ; // MOV EAX
                } 
                else if(strcmp(token_registros, "r_base32") == 0) {
                    fputc(0xBB, archivo_escritura) ; // MOV EBX
                }
                else if(strcmp(token_registros, "r_contador32") == 0) {
                    fputc(0xB9, archivo_escritura) ; // MOV ECX, imm32
                }
                else if(strcmp(token_registros, "r_datos32") == 0) {
                    fputc(0xBA, archivo_escritura) ; // MOV EDX, imm32
                }
                else if(strcmp(token_registros, "r_puntero32") == 0) {
                    fputc(0xBE, archivo_escritura) ; // MOV ESI, imm32
                }
                else if(strcmp(token_registros, "r_destino32") == 0) {
                    fputc(0xBF, archivo_escritura) ; // MOV EDI, imm32
                }
                else if(strcmp(token_registros, "r_pila32") == 0) {
                    fputc(0xBC, archivo_escritura) ; // MOV ESP, imm32
                }
                // Registros especiales de 32 bits
                else if(strcmp(token_registros, "r8_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB8, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xB9, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xBA, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xBB, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xBC, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xBD, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xBE, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_32") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0xBF, archivo_escritura); 
                }
                // Registros clasicos de 64 bits
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
                // Registros especiales de 64 bits
                else if(strcmp(token_registros, "r8_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0xB8, archivo_escritura); 
                    es_64 = 1; 
                }
                else if(strcmp(token_registros, "r9_64") == 0) { 
                    fputc(0x49, archivo_escritura); 
                    fputc(0xB9, archivo_escritura); 
                    es_64 = 1; 
                }
                else if(strcmp(token_registros, "r10_64") == 0) { 
                    fputc(0x49, archivo_escritura); 
                    fputc(0xBA, archivo_escritura); 
                    es_64 = 1; 
                }
                else if(strcmp(token_registros, "r11_64") == 0) { 
                    fputc(0x49, archivo_escritura); 
                    fputc(0xBB, archivo_escritura); 
                    es_64 = 1; 
                }
                else if(strcmp(token_registros, "r12_64") == 0) { 
                    fputc(0x49, archivo_escritura); 
                    fputc(0xBC, archivo_escritura); 
                    es_64 = 1; 
                }
                else if(strcmp(token_registros, "r13_64") == 0) { 
                    fputc(0x49, archivo_escritura); 
                    fputc(0xBD, archivo_escritura); 
                    es_64 = 1; 
                }
                else if(strcmp(token_registros, "r14_64") == 0) { 
                    fputc(0x49, archivo_escritura); 
                    fputc(0xBE, archivo_escritura); 
                    es_64 = 1; 
                }
                else if(strcmp(token_registros, "r15_64") == 0) { 
                    fputc(0x49, archivo_escritura); 
                    fputc(0xBF, archivo_escritura); 
                    es_64 = 1; 
                }
                if (strstr(token_registros, "64") != NULL) {
                    long long numerob = atoll(token_numero) ;
                    fwrite(&numerob, sizeof(long long), 1, archivo_escritura) ; // 8 bytes
                } 
                else if (strstr(token_registros, "32") != NULL) {
                    int numero = atoi(token_numero) ;
                    fwrite(&numero, sizeof(int), 1, archivo_escritura) ; // 4 bytes
                }
                else if (strstr(token_registros, "16") != NULL) {
                    short numero = (short)atoi(token_numero) ;
                    fwrite(&numero, sizeof(short), 1, archivo_escritura) ; // 2 bytes
                }
                else if (strstr(token_registros, "8") != NULL) {
                    char numero = (char)atoi(token_numero) ;
                    fputc(numero, archivo_escritura) ; // 1 byte
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
            
            // --- ATAJO MATEMÁTICO PARA REGISTROS NUEVOS (RESTAR) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                if (id_origen != -1 && id_destino != -1 && tam_origen == tam_destino) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08; // REX.W
                    if (id_origen >= 8) rex |= 0x04; // REX.R
                    if (id_destino >= 8) rex |= 0x01; // REX.B

                    int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);
                    
                    // 0x28 para SUB de 8 bits, 0x29 para el resto
                    int opcode = (tam_destino == 8) ? 0x28 : 0x29; 

                    if (tam_destino == 16) fputc(0x66, archivo_escritura); 
                    if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura); 
                    
                    fputc(opcode, archivo_escritura);
                    fputc(modrm, archivo_escritura);
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---
            
            if(isdigit(token_numero[0])) {
                // --- REGISTROS CLÁSICOS DE 8 BITS ---
                if(strcmp(token_registros, "r_acumulador8") == 0) { fputc(0x2C, archivo_escritura); } // SUB AL, imm8
                else if(strcmp(token_registros, "r_base8") == 0) { fputc(0x80, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador8") == 0) { fputc(0x80, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos8") == 0) { fputc(0x80, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // --- REGISTROS ESPECIALES DE 8 BITS ---
                else if(strcmp(token_registros, "r8_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // --- REGISTROS CLÁSICOS DE 16 BITS ---
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x2D, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // --- REGISTROS ESPECIALES DE 16 BITS ---
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // --- REGISTROS CLÁSICOS DE 32 BITS ---
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0x2D, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0x81, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0x81, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0x81, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0x81, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0x81, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0x81, archivo_escritura); fputc(0xEF, archivo_escritura); }

                // --- REGISTROS ESPECIALES DE 32 BITS ---
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEF, archivo_escritura); }
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
                else if(strcmp(token_registros, "r8_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE8, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xE9, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEA, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEB, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEC, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xED, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEE, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xEF, archivo_escritura); 
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
            
            // --- ATAJO MATEMÁTICO PARA REGISTROS NUEVOS (COMPARAR) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                if (id_origen != -1 && id_destino != -1 && tam_origen == tam_destino) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08; // REX.W
                    if (id_origen >= 8) rex |= 0x04; // REX.R
                    if (id_destino >= 8) rex |= 0x01; // REX.B

                    int modrm = 0xC0 | ((id_origen & 7) << 3) | (id_destino & 7);
                    
                    // 0x38 para CMP de 8 bits, 0x39 para el resto
                    int opcode = (tam_destino == 8) ? 0x38 : 0x39; 

                    if (tam_destino == 16) fputc(0x66, archivo_escritura); 
                    if (rex != 0x40 || tam_destino == 8) fputc(rex, archivo_escritura); 
                    
                    fputc(opcode, archivo_escritura);
                    fputc(modrm, archivo_escritura);
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---
            
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO
                // --- REGISTROS CLÁSICOS DE 8 BITS ---
                if(strcmp(token_registros, "r_acumulador8") == 0) { fputc(0x3C, archivo_escritura); } // CMP AL, imm8
                else if(strcmp(token_registros, "r_base8") == 0) { fputc(0x80, archivo_escritura); fputc(0xFB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador8") == 0) { fputc(0x80, archivo_escritura); fputc(0xF9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos8") == 0) { fputc(0x80, archivo_escritura); fputc(0xFA, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino8") == 0) { fputc(0x40, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFF, archivo_escritura); }
                // --- REGISTROS ESPECIALES DE 8 BITS ---
                else if(strcmp(token_registros, "r8_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xF9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFD, archivo_escritura); }
                else if(strcmp(token_registros, "r14_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_8") == 0) { fputc(0x41, archivo_escritura); fputc(0x80, archivo_escritura); fputc(0xFF, archivo_escritura); }

                // --- REGISTROS CLÁSICOS DE 16 BITS ---
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x3D, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFA, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFF, archivo_escritura); }
                // --- REGISTROS ESPECIALES DE 16 BITS ---
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFD, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFF, archivo_escritura); }

                // --- REGISTROS CLÁSICOS DE 32 BITS ---
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0x3D, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0x81, archivo_escritura); fputc(0xFB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0x81, archivo_escritura); fputc(0xF9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0x81, archivo_escritura); fputc(0xFA, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0x81, archivo_escritura); fputc(0xFC, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0x81, archivo_escritura); fputc(0xFE, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0x81, archivo_escritura); fputc(0xFF, archivo_escritura); }
                // --- REGISTROS ESPECIALES DE 32 BITS ---
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF8, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFA, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFC, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFD, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFE, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x41, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFF, archivo_escritura); }

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
                // --- NUEVOS REGISTROS r8_64 a r15_64 (INMEDIATO COMPARAR) ---
                else if(strcmp(token_registros, "r8_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF8, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xF9, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFA, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFB, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFC, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFD, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFE, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_64") == 0) { 
                    fputc(0x49, archivo_escritura); fputc(0x81, archivo_escritura); fputc(0xFF, archivo_escritura); 
                }
                if (strstr(token_registros, "16") != NULL) {
                    short numero = (short)atoi(token_numero);
                    fwrite(&numero, sizeof(short), 1, archivo_escritura); // 2 bytes
                } else if (strstr(token_registros, "8") != NULL) {
                    char numero = (char)atoi(token_numero);
                    fputc(numero, archivo_escritura); // 1 byte
                } else {
                    int numero = atoi(token_numero);
                    fwrite(&numero, sizeof(int), 1, archivo_escritura); // 4 bytes (32 y 64 bits)
                }
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
            
            // --- ATAJO MATEMÁTICO PARA REGISTROS NUEVOS (MULTIPLICAR) ---
            if (!isdigit(token_numero[0])) {
                int id_origen = obtener_id_registro(token_numero);
                int id_destino = obtener_id_registro(token_registros);
                int tam_origen = obtener_tamano_registro(token_numero);
                int tam_destino = obtener_tamano_registro(token_registros);

                // imul solo soporta 16, 32 y 64 bits en modo de 2 operandos
                if (id_origen != -1 && id_destino != -1 && tam_origen == tam_destino && tam_destino >= 16) {
                    int rex = 0x40;
                    if (tam_destino == 64) rex |= 0x08; // REX.W
                    if (id_destino >= 8) rex |= 0x04; // REX.R (para el destino en Reg)
                    if (id_origen >= 8) rex |= 0x01; // REX.B (para el origen en R/M)

                    // ¡ATENCIÓN AL ORDEN! Destino en el medio (Reg), Origen en la derecha (R/M)
                    int modrm = 0xC0 | ((id_destino & 7) << 3) | (id_origen & 7);
                    
                    // Prefijo para 16 bits
                    if (tam_destino == 16) fputc(0x66, archivo_escritura); 
                    
                    if (rex != 0x40) fputc(rex, archivo_escritura); 
                    
                    fputc(0x0F, archivo_escritura);
                    fputc(0xAF, archivo_escritura);
                    fputc(modrm, archivo_escritura);
                    continue;
                }
            }
            // --- FIN DEL ATAJO ---
            
            if(isdigit(token_numero[0])) {
                // MODO INMEDIATO (imul reg, reg, imm32)
                
                // --- REGISTROS CLÁSICOS DE 8 BITS (IMUL CORREGIDO) ---
                // (MOVZX EAX, registro; IMUL EAX, EAX, imm32; MOV registro, AL)
                if(strcmp(token_registros, "r_acumulador8") == 0) { 
                    // Para AL es más directo: IMUL EAX, EAX, imm32
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_base8") == 0) { 
                    fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC3, archivo_escritura); // MOVZX EAX, BL
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); // IMUL EAX, EAX, imm32
                    fputc(0x88, archivo_escritura); fputc(0xC3, archivo_escritura); // MOV BL, AL
                }
                else if(strcmp(token_registros, "r_contador8") == 0) { 
                    fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC1, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x88, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_datos8") == 0) { 
                    fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC2, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x88, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r_pila8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC4, archivo_escritura); // MOVZX EAX, SPL
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x40, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC4, archivo_escritura); // MOV SPL, AL
                }
                else if(strcmp(token_registros, "r_puntero8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC6, archivo_escritura); // MOVZX EAX, SIL
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x40, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC6, archivo_escritura); // MOV SIL, AL
                }
                else if(strcmp(token_registros, "r_destino8") == 0) { 
                    fputc(0x40, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC7, archivo_escritura); // MOVZX EAX, DIL
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x40, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC7, archivo_escritura); // MOV DIL, AL
                }

                // --- REGISTROS ESPECIALES DE 8 BITS (IMUL CORREGIDO) ---
                // (MOVZX EAX, r8b -> 41 0F B6 C0, etc)
                else if(strcmp(token_registros, "r8_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC1, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC1, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC2, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC3, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC3, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC4, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC5, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC5, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC6, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_8") == 0) { 
                    fputc(0x41, archivo_escritura); fputc(0x0F, archivo_escritura); fputc(0xB6, archivo_escritura); fputc(0xC7, archivo_escritura); 
                    fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                    fputc(0x41, archivo_escritura); fputc(0x88, archivo_escritura); fputc(0xC7, archivo_escritura); 
                }
                // --- REGISTROS CLÁSICOS DE 16 BITS (IMUL) ---
                else if(strcmp(token_registros, "r_acumulador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); }
                else if(strcmp(token_registros, "r_base16") == 0) { fputc(0x66, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xDB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador16") == 0) { fputc(0x66, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos16") == 0) { fputc(0x66, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xD2, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila16") == 0) { fputc(0x66, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero16") == 0) { fputc(0x66, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino16") == 0) { fputc(0x66, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xFF, archivo_escritura); }

                // --- REGISTROS ESPECIALES DE 16 BITS (IMUL) ---
                else if(strcmp(token_registros, "r8_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xD2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xDB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_16") == 0) { fputc(0x66, archivo_escritura); fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xFF, archivo_escritura); }

                // --- REGISTROS CLÁSICOS DE 32 BITS (IMUL) ---
                else if(strcmp(token_registros, "r_acumulador32") == 0) { fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); }
                else if(strcmp(token_registros, "r_base32") == 0) { fputc(0x69, archivo_escritura); fputc(0xDB, archivo_escritura); }
                else if(strcmp(token_registros, "r_contador32") == 0) { fputc(0x69, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r_datos32") == 0) { fputc(0x69, archivo_escritura); fputc(0xD2, archivo_escritura); }
                else if(strcmp(token_registros, "r_pila32") == 0) { fputc(0x69, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r_puntero32") == 0) { fputc(0x69, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r_destino32") == 0) { fputc(0x69, archivo_escritura); fputc(0xFF, archivo_escritura); }

                // --- REGISTROS ESPECIALES DE 32 BITS (IMUL) ---
                else if(strcmp(token_registros, "r8_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); }
                else if(strcmp(token_registros, "r9_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC9, archivo_escritura); }
                else if(strcmp(token_registros, "r10_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xD2, archivo_escritura); }
                else if(strcmp(token_registros, "r11_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xDB, archivo_escritura); }
                else if(strcmp(token_registros, "r12_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xE4, archivo_escritura); }
                else if(strcmp(token_registros, "r13_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xED, archivo_escritura); }
                else if(strcmp(token_registros, "r14_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xF6, archivo_escritura); }
                else if(strcmp(token_registros, "r15_32") == 0) { fputc(0x45, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xFF, archivo_escritura); }

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
                // --- NUEVOS REGISTROS r8_64 a r15_64 (INMEDIATO MULTIPLICAR) ---
                else if(strcmp(token_registros, "r8_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC0, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r9_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xC9, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r10_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xD2, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r11_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xDB, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r12_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xE4, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r13_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xED, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r14_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xF6, archivo_escritura); 
                }
                else if(strcmp(token_registros, "r15_64") == 0) { 
                    fputc(0x4D, archivo_escritura); fputc(0x69, archivo_escritura); fputc(0xFF, archivo_escritura); 
                }
                // Escribir el inmediato con el tamaño correcto
                if (strstr(token_registros, "16") != NULL) {
                    short numero = (short)atoi(token_numero);
                    fwrite(&numero, sizeof(short), 1, archivo_escritura); // 2 bytes
                } else {
                    // Para 64, 32 y 8 bits usamos 4 bytes (imm32), 
                    // porque el de 8 bits lo hemos extendido a EAX
                    int numero = atoi(token_numero);
                    fwrite(&numero, sizeof(int), 1, archivo_escritura); // 4 bytes
                }
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
            // (Ojo: para 8 bits el dividendo es AX, y para 16 bits es DX:AX. 
            // El XOR RDX, RDX limpia DX y EDX, así que funciona bien).
            
            // --- 64 BITS ---
            if(strcmp(token_numero, "r_acumulador64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura); // XOR RDX, RDX
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV RAX
            }
            else if(strcmp(token_numero, "r_base64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV RBX
            }
            else if(strcmp(token_numero, "r_contador64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV RCX
            }
            else if(strcmp(token_numero, "r_datos64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV RDX
            }
            else if(strcmp(token_numero, "r_puntero64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV RSI
            }
            else if(strcmp(token_numero, "r_destino64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV RDI
            }
            else if(strcmp(token_numero, "r_pila64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x48, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV RSP
            }
            else if(strcmp(token_numero, "r8_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV R8
            }
            else if(strcmp(token_numero, "r9_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV R9
            }
            else if(strcmp(token_numero, "r10_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV R10
            }
            else if(strcmp(token_numero, "r11_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV R11
            }
            else if(strcmp(token_numero, "r12_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV R12
            }
            else if(strcmp(token_numero, "r13_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF5, archivo_escritura); // DIV R13
            }
            else if(strcmp(token_numero, "r14_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV R14
            }
            else if(strcmp(token_numero, "r15_64") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x49, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV R15
            }
            // --- 32 BITS ---
            else if(strcmp(token_numero, "r_acumulador32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0xF7, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV EAX
            }
            else if(strcmp(token_numero, "r_base32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0xF7, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV EBX
            }
            else if(strcmp(token_numero, "r_contador32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0xF7, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV ECX
            }
            else if(strcmp(token_numero, "r_datos32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0xF7, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV EDX
            }
            else if(strcmp(token_numero, "r_pila32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0xF7, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV ESP
            }
            else if(strcmp(token_numero, "r_puntero32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0xF7, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV ESI
            }
            else if(strcmp(token_numero, "r_destino32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0xF7, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV EDI
            }
            else if(strcmp(token_numero, "r8_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV R8D
            }
            else if(strcmp(token_numero, "r9_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV R9D
            }
            else if(strcmp(token_numero, "r10_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV R10D
            }
            else if(strcmp(token_numero, "r11_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV R11D
            }
            else if(strcmp(token_numero, "r12_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV R12D
            }
            else if(strcmp(token_numero, "r13_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF5, archivo_escritura); // DIV R13D
            }
            else if(strcmp(token_numero, "r14_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV R14D
            }
            else if(strcmp(token_numero, "r15_32") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV R15D
            }
            // --- 16 BITS ---
            else if(strcmp(token_numero, "r_acumulador16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV AX
            }
            else if(strcmp(token_numero, "r_base16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV BX
            }
            else if(strcmp(token_numero, "r_contador16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV CX
            }
            else if(strcmp(token_numero, "r_datos16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV DX
            }
            else if(strcmp(token_numero, "r_pila16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV SP
            }
            else if(strcmp(token_numero, "r_puntero16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV SI
            }
            else if(strcmp(token_numero, "r_destino16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV DI
            }
            else if(strcmp(token_numero, "r8_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV R8W
            }
            else if(strcmp(token_numero, "r9_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV R9W
            }
            else if(strcmp(token_numero, "r10_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV R10W
            }
            else if(strcmp(token_numero, "r11_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV R11W
            }
            else if(strcmp(token_numero, "r12_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV R12W
            }
            else if(strcmp(token_numero, "r13_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF5, archivo_escritura); // DIV R13W
            }
            else if(strcmp(token_numero, "r14_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV R14W
            }
            else if(strcmp(token_numero, "r15_16") == 0) {
                fputc(0x48, archivo_escritura); fputc(0x31, archivo_escritura); fputc(0xD2, archivo_escritura);
                fputc(0x66, archivo_escritura); fputc(0x41, archivo_escritura); fputc(0xF7, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV R15W
            }
            // --- 8 BITS ---
            else if(strcmp(token_numero, "r_acumulador8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0xF6, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV AL
            }
            else if(strcmp(token_numero, "r_base8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0xF6, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV BL
            }
            else if(strcmp(token_numero, "r_contador8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0xF6, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV CL
            }
            else if(strcmp(token_numero, "r_datos8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0xF6, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV DL
            }
            else if(strcmp(token_numero, "r_pila8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x40, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV SPL
            }
            else if(strcmp(token_numero, "r_puntero8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x40, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV SIL
            }
            else if(strcmp(token_numero, "r_destino8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x40, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV DIL
            }
            else if(strcmp(token_numero, "r8_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF0, archivo_escritura); // DIV R8B
            }
            else if(strcmp(token_numero, "r9_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF1, archivo_escritura); // DIV R9B
            }
            else if(strcmp(token_numero, "r10_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF2, archivo_escritura); // DIV R10B
            }
            else if(strcmp(token_numero, "r11_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF3, archivo_escritura); // DIV R11B
            }
            else if(strcmp(token_numero, "r12_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF4, archivo_escritura); // DIV R12B
            }
            else if(strcmp(token_numero, "r13_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF5, archivo_escritura); // DIV R13B
            }
            else if(strcmp(token_numero, "r14_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF6, archivo_escritura); // DIV R14B
            }
            else if(strcmp(token_numero, "r15_8") == 0) {
                fputc(0x30, archivo_escritura); fputc(0xE4, archivo_escritura); // XOR AH, AH
                fputc(0x41, archivo_escritura); fputc(0xF6, archivo_escritura); fputc(0xF7, archivo_escritura); // DIV R15B
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