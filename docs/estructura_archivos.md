# Estructura de Archivos y Arquitectura de ENSMES

Este documento detalla la función de cada archivo y carpeta en el ecosistema ENSMES, así como la arquitectura de sus componentes principales.

## 1. Código Fuente del Compilador y Cargador (Carpeta `src/`)

*   **`main.c`**: Es el punto de entrada de la herramienta. Se encarga de parsear los argumentos de la terminal (los comandos `compilar`, `ejecutar`, `version`, etc.) y de llamar a las funciones correspondientes.
*   **`ensamblador.c`**: Es el motor de traducción. Contiene la lógica de tokenización, el mapeo de opcodes, el sistema de etiquetas (backpatching) y la generación del formato ejecutable JMA. También aloja el cargador (loader) en C que ejecuta los archivos `.jma` en memoria.
*   **`ensamblador.h`**: Es el contrato del motor. Define las estructuras de datos compartidas (Etiquetas, Saltos) y los prototipos de las funciones que `main.c` necesita conocer.

## 2. Archivos de Entrada y Salida (Carpeta `ejemplos/`)

*   **`*.ensmes`**: Son los archivos de código fuente escritos en el lenguaje ENSMES. Aquí escribes las instrucciones (`copiar`, `sumar`, etc.).
*   **`*.jma`**: Es el archivo binario ejecutable generado por el compilador. Contiene la cabecera JMA (80 bytes) seguida del código y los datos traducidos a lenguaje máquina x86_64. 
    *   *Nota de arquitectura:* El orden de las secciones `.datos` y `.codigo` es libre. El compilador calcula dinámicamente los offsets en la cabecera para que el cargador sepa dónde está cada cosa, sin importar cuál se escribió primero.

## 3. Documentación (Carpeta `docs/`)

*   **`DOCUMENTACION.md`**: El documento principal que explica el propósito, la visión y la arquitectura general del proyecto.
*   **`estructura_archivos.md`**: Este documento.
*   **`comandos_ens.md`**: Manual de uso de la interfaz de línea de comandos (la herramienta `ens`).
*   **`instrucciones.md`**: Referencia técnica completa del lenguaje (registros y opcodes).

## 4. Personalización del Editor (Carpeta `ensmes-icons/`)

*   Contiene todos los archivos `.json` y `.svg` necesarios para el tema de iconos personalizado de ENSMES para VS Code / VSCodium.

## 5. Archivos en la Raíz del Proyecto

*   **`README.md`**: La portada del repositorio. Explica qué es ENSMES, sus características y su roadmap.
*   **`.gitignore`**: Archivo de configuración de Git que evita que se suban al repositorio los binarios compilados (como `ens`, `ensamblador`) y los archivos `.jma` generados.
*   **`Makefile`** *(opcional)*: Si se añade, simplifica la compilación del código fuente en C con un solo comando (`make`).