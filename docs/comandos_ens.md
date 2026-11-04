# Manual de Comandos: `ens`

La herramienta `ens` (Entorno ENSMES) es la interfaz de línea de comandos para compilar y ejecutar programas escritos en ENSMES.

## Sintaxis Básica
```bash
ens <comando> [argumentos]

Comandos Disponibles
1. compilar
Uso: ens compilar <archivo.ensmes>

Descripción: Lee el archivo fuente en ENSMES, traduce las instrucciones a código máquina x86_64 y genera un archivo ejecutable con la extensión .jma en la misma carpeta.

Ejemplo:

bash
ens compilar mi_programa.ensmes
# Genera: mi_programa.jma
2. ejecutar
Uso: ens ejecutar <archivo.jma>

Descripción: Abre el archivo JMA, verifica que la firma mágica sea correcta, carga el código en memoria y lo ejecuta en la CPU. Es el cargador oficial del imperio.

Ejemplo:

bash
ens ejecutar mi_programa.jma
3. version
Uso: ens version

Descripción: Muestra la versión actual de la herramienta y del formato.

Ejemplo:

bash
ens version
# Salida: ens v1.0 - ENSMES
4. ayuda
Uso: ens ayuda

Descripción: Muestra un menú con la lista de comandos disponibles y su sintaxis.

Ejemplo:

ens ayuda

Flujo de Trabajo Recomendado:

Escribir el código en un archivo .ensmes.

Compilar: ens compilar programa.ensmes.

Ejecutar: ens ejecutar programa.jma.