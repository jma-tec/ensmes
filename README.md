# ENSMES 🚀
**El primer ensamblador x86_64 con sintaxis en español, diseñado para la Soberanía Tecnológica.**

ENSMES es un ensamblador modular escrito en C que genera ejecutables en un formato propio (`.jma`). Nace con el objetivo de ser la base de un ecosistema completo de programación, abarcando desde un lenguaje de alto nivel hasta un Kernel y Sistema Operativo propio de propósito general.

## Características actuales
- Sintaxis legible en español (`copiar`, `sumar`, `saltar_si_igual`).
- Formato de ejecutable propio (`.jma`) con cargador integrado usando `mmap` en direcciones fijas.
- Soporte para operaciones aritméticas, lógicas y de control de flujo.
- Manejo de memoria: acceso directo, indirecto y con offsets (`[registro + offset]`).
- Soporte **en desarrollo** para registros de 8, 32 y 64 bits (siendo el ecosistema de 64 bits el núcleo actual y más maduro).
- Total libertad en el orden de las secciones (`.datos` y `.codigo`).

## Ejemplo de código (Bucle en ENSMES)
```text
.datos
letra_a:
definir_byte 65 # 'A'

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

```


## Roadmap: El Ecosistema JMA
☑ Ensamblador base y cargador en C.

☑ Soporte de control de flujo, memoria y offsets.

□ Lenguaje de alto nivel.

□ Kernel y Sistema Operativo propio de propósito general.

## Compilación y Uso
Para compilar el ensamblador en Linux (asegúrate de tener gcc instalado):

bash
gcc src/main.c src/ensamblador.c -o ens
Para compilar un programa .ensmes:

bash
./ens compilar mi_programa.ensmes
Para ejecutarlo:

bash
./ens ejecutar mi_programa.jma

## ⚠️ AVISO IMPORTANTE: Estado del Proyecto y Transparencia
Quiero ser totalmente honesto con cualquiera que se acerque a este repositorio. ENSMES es un proyecto en desarrollo temprano y experimental.

El código puede tener errores: Aunque el núcleo funcional (compilador y cargador) ha sido probado y funciona, es muy probable que existan bugs ocultos, casos límite no cubiertos o comportamientos inesperados.

La documentación puede estar incompleta o ser imprecisa: Aunque he intentado documentar todo lo posible, es posible que falten detalles sobre ciertos opcodes, que haya instrucciones documentadas de forma incompleta o que algunos matices técnicos estén desactualizados con respecto al código.

Esto no es un producto terminado: Estoy aprendiendo y expandiendo mis conocimientos de arquitectura de computadores y desarrollo de compiladores a medida que construyo ENSMES.

Si encuentras algún error, una instrucción mal documentada, un fallo de compilación o simplemente algo que no cuadra, te agradecería enormemente que lo notificaras. Puedes abrir un Issue en este repositorio o enviar una sugerencia. La crítica constructiva y la ayuda de la comunidad son fundamentales para que este ecosistema crezca y sea robusto.

## 🤝 Contribuciones
Si compartes la visión de crear tecnología nativa en español y de bajo nivel, ¡las ideas, reportes de bugs, críticas y sugerencias son más que bienvenidas!
