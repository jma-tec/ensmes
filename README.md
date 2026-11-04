# ENSMES
**El primer ensamblador x86_64 con sintaxis en español, diseñado para la Soberanía Tecnológica.**

ENSMES es un ensamblador modular escrito en C que genera ejecutables en un formato propio (`.jma`). Nace con el objetivo de ser la base de un ecosistema completo de programación, abarcando desde un lenguaje de alto nivel hasta un Kernel y Sistema Operativo propio de propósito general.

## Características
- Sintaxis legible en español (`copiar`, `sumar`, `saltar_si_igual`).
- Formato de ejecutable propio (`.jma`) con cargador integrado usando `mmap` en direcciones fijas.
- Soporte para operaciones aritméticas, lógicas y de control de flujo.
- Manejo avanzado de memoria: acceso directo, indirecto y con offsets (`[registro + offset]`).
- Soporte para registros de 8, 32 y 64 bits.
- Total libertad en el orden de las secciones (`.datos` y `.codigo`).

## Ejemplo de código (Bucle en ENSMES)
```text
.codigo
inicio:
copiar 5, r_base64
bucle:
comparar 0, r_base64
saltar_si_igual fin
cargar_direccion letra_a, r_puntero64
copiar 1, r_datos64
copiar 1, r_acumulador64
copiar 1, r_destino64
interrumpir
restar 1, r_base64
saltar bucle
fin:
terminar
.datos
letra_a:
definir_byte 65 # 'A'