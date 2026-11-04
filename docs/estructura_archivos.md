# Estructura de Archivos y Arquitectura de ENSMES

Este documento detalla la función de cada archivo en el ecosistema ENSMES.

## 1. Código Fuente del Compilador (C)
*   **`main.c`**: Es el punto de entrada de la herramienta. Se encarga de parsear los argumentos de la terminal (los comandos `compilar`, `ejecutar`, etc.) y de llamar a las funciones correspondientes. También gestiona la apertura y cierre de los archivos de entrada y salida.
*   **`ensamblador.c`**: Es el motor de traducción. Contiene la lógica de tokenización, el mapeo de opcodes, el sistema de etiquetas (backpatching) y la generación del formato ejecutable JMA. También aloja el cargador (loader) en C que ejecuta los archivos `.jma`.
*   **`ensamblador.h`**: Es el contrato del motor. Define las estructuras de datos (Etiquetas, Saltos), las variables globales y los prototipos de las funciones que `main.c` necesita conocer.

## 2. Archivos de Entrada y Salida
*   **`programa.ensmes`**: Es el archivo de código fuente escrito en el lenguaje ENSMES. Es el archivo que tú escribes con las instrucciones (`copiar`, `sumar`, etc.).
*   **`programa.jma`**: Es el archivo binario ejecutable generado por el compilador. Contiene la cabecera JMA (80 bytes) seguida del código y los datos traducidos a lenguaje máquina x86_64.

## 3. Herramientas del Sistema
*   **`ens`**: Es el ejecutable final de la herramienta, generado al compilar `main.c` y `ensamblador.c` con `gcc`. Es el comando que usarás en la terminal.

## 4. Documentación
*   **`estructura_archivos.md`**: Este documento.
*   **`comandos_ens.md`**: Manual de uso de los comandos de la terminal.
*   **`instrucciones.md`**: Referencia técnica de los opcodes y sintaxis del lenguaje.
