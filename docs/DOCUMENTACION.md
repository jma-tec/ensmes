## Titulo
Ensamblador Modular Español

## Propósito
El propósito de este proyecto es crear herramientas hispanas nativas para fomentar nuestra propia lengua en la creación de software y hardware.

## Arquitectura del sistema

1. **`main.c`**: Actúa como el punto de entrada y orquestador del sistema, encargándose de la inicialización y validación de los recursos E/S (archivos).

2. **`ensamblador.c`**: Se encarga de la lógica de negocio y el motor de traducción. Realiza la tokenización del flujo de texto, el mapeo de códigos de operación a escritura binaria y la carga en memoria. Genera el formato ejecutable propio ".jma".

3. **`ensamblador.h`**: Define la interfaz pública del motor, exponiendo los prototipos de las funciones y estableciendo los contratos entre módulos.