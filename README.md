# ENSMES 🚀
**El primer ensamblador x86_64 con sintaxis en español, diseñado para la Soberanía Tecnológica.**

ENSMES es un ensamblador modular escrito en C que genera ejecutables en un formato propio (`.jma`). Nace con el objetivo de ser la base de un ecosistema completo de programación (Corporación JMA), abarcando desde un lenguaje de alto nivel hasta un Kernel y Sistema Operativo propio de propósito general.

## Características actuales
- **Sintaxis legible en español** (`copiar`, `sumar`, `saltar_si_igual`, `desplazar_izquierda`).
- **Formato de ejecutable propio** (`.jma`) con cargador integrado usando `mmap` en direcciones fijas.
- **ISA (Instruction Set Architecture) completa**: Soporte total para operaciones aritméticas (`sumar`, `restar`, `multiplicar`, `dividir`), lógicas (`y`, `o`, `o_exclusivo`) y desplazamientos.
- **Soporte de registros total**: Todos los registros clásicos y especiales (`r8` a `r15`) en sus 4 tamaños (8, 16, 32 y 64 bits).
- **Manejo avanzado de memoria**:
  - Acceso directo por etiqueta.
  - Acceso indirecto puro (`[registro]`).
  - Acceso indirecto con offset (`[registro + offset]`).
  - Cálculo completo de **ModR/M y SIB**, permitiendo usar cualquier registro base, incluyendo `r12` y `rsp`.
- **Gestión de Pila**: Instrucciones `empujar` y `sacar` para todos los registros y tamaños.
- **Modo Inmediato y Registro**: Las operaciones aceptan tanto valores inmediatos como paso de registro a registro.
- **Total libertad** en el orden de las secciones (`.datos` y `.codigo`).

## Ejemplo de código (Memoria Indirecta y Registros Especiales)

    .datos
    buffer:
    reservar_byte 16

    .codigo
    inicio:
        cargar_direccion buffer, r_puntero64

        # Escribir 5 en [r12_64] (Fuerza el uso de byte SIB)
        copiar 5, r_acumulador8
        guardar_puntero r_acumulador8, [r12_64]

        # Leer de [r_puntero64] y sumar 1
        cargar [r_puntero64], r15_64
        sumar 1, r15_64

        # Imprimir el resultado (6) por pantalla
        copiar r15_64, r_acumulador64
        sumar 48, r_acumulador64
        guardar_byte_puntero r_acumulador64, [r_puntero64]
        
        cargar_direccion buffer, r_puntero64
        copiar 1, r_datos64
        copiar 1, r_acumulador64
        copiar 1, r_destino64
        interrumpir

        terminar

## Roadmap: El Ecosistema JMA

- [x] Ensamblador base y cargador en C.
- [x] Soporte de control de flujo, memoria y offsets.
- [x] **ISA Aritmética y Lógica completa (8, 16, 32, 64 bits).**
- [x] **Soporte completo de pila (Push/Pop).**
- [x] **Acceso a memoria indirecto complejo (SIB).**
- [ ] Lenguaje de alto nivel (en desarrollo).
- [ ] Kernel y Sistema Operativo propio de propósito general.

## Compilación y Uso

Para compilar el ensamblador en Linux (asegúrate de tener `gcc` instalado):

    gcc src/main.c src/ensamblador.c -o ens

Para compilar un programa `.ensmes`:

    ./ens compilar mi_programa.ensmes

Para ejecutarlo:

    ./ens ejecutar mi_programa.jma

## Estado del Proyecto y Transparencia

> **Este es un proyecto en desarrollo temprano y experimental.**
> 
> Quiero ser totalmente honesto con cualquiera que se acerque a este repositorio:
> 
> - **El código puede tener errores:** Aunque el núcleo funcional (compilador y cargador) ha sido probado y la ISA es estable, es probable que existan bugs ocultos o casos límite no cubiertos.
> - **La documentación puede estar incompleta:** Es posible que falten detalles sobre ciertos opcodes o que algunos matices técnicos estén desactualizados.
> - **Evolución constante:** Estoy aprendiendo y expandiendo mis conocimientos de arquitectura de computadores a medida que construyo ENSMES.
> 
> *Si encuentras algún error, una instrucción mal documentada o algo que no cuadra, agradecería enormemente que lo notificaras.*

## Contribuciones

Actualmente, el proyecto se encuentra en una fase temprana de desarrollo. Sin embargo, si compartes la visión de crear tecnología nativa en español y de bajo nivel, **tu ayuda es más que bienvenida**. 

Puedes contribuir de las siguientes formas:
- **Reportando bugs:** Abriendo un *Issue* si algo no funciona como debería.
- **Sugiriendo mejoras:** Proponiendo nuevas instrucciones o mejoras en la arquitectura.
- **Mejorando la documentación:** Si ves algo que falta o está mal explicado, ¡envía un *Pull Request*!