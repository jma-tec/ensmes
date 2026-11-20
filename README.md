# ENSMES 🚀
**El primer ensamblador x86_64 con sintaxis en español, diseñado para la Soberanía Tecnológica.**

ENSMES es un ensamblador modular escrito en C que genera ejecutables en formato propio `.jma`. Nace como base del ecosistema JMA, abarcando desde lenguajes de alto nivel hasta Kernel y SO propios.

## Características actuales (v2)
- **Sintaxis legible en español**: `copiar`, `sumar`, `saltar_si_igual`, `desplazar_izquierda`, etc.
- **Formato JMA v2 profesional**: Cabecera estructurada, secciones explícitas (.datos, .constantes, .codigo, .reservado) y alineación automática a páginas de 4096 bytes para soporte seguro de W^X.
- **Cargador dinámico avanzado**: Usa `mmap` con ASLR (sin direcciones fijas), gestiona BSS y asigna Heap dinámico independiente (1MB por defecto) pasado al programa vía segmento de datos.
- **ISA completa**: Aritmética (`sumar`, `restar`, `multiplicar`, `dividir`), lógica (`y`, `o`, `o_exclusivo`) y desplazamientos en 8/16/32/64 bits.
- **Registros totales**: Clásicos + especiales (r8-r15) en todos los tamaños.
- **Memoria avanzada**: Acceso directo, indirecto `[reg]`, con offset `[reg+offset]` y cálculo completo ModR/M+SIB.
- **Pila completa**: `empujar` / `sacar` para todos los registros.
- **Flexibilidad total**: Orden libre de secciones y resolución de etiquetas en dos pasadas.
- **Registros vectoriales de 128 bits**: `xmm0_128-xmm15_128`
- **Instrucciones vectoriales para 128 bits**
### Ejemplo de código (Heap Dinámico y Memoria Indirecta)

```ensmes
.datos
    heap_ptr:
        reservar_byte 8      # El loader escribe aquí la dirección del heap
    buffer:
        reservar_byte 16

.codigo
inicio:
    # Leer puntero al heap desde [heap_ptr]
    cargar_direccion heap_ptr, r_puntero64
    cargar [r_puntero64], r12_64
    
    # Usar buffer para imprimir resultado
    cargar_direccion buffer, r_puntero64
    copiar 72, r_acumulador8   # 'H'
    guardar_byte_puntero r_acumulador8, [r_puntero64]
    
    # write(1, buffer, 1)
    cargar_direccion buffer, r_puntero64
    copiar 1, r_datos64
    copiar 1, r_destino64
    copiar 1, r_acumulador64
    interrumpir
    
    terminar
```

## Roadmap: Ecosistema JMA

- [x] Ensamblador base y cargador en C
- [x] ISA aritmética, lógica y pila completa
- [x] Memoria indirecta compleja (SIB) y acceso a registros r8-r15
- [x] **Formato JMA v2 con alineación de páginas y Heap dinámico**
- [ ] Lenguaje de alto nivel Neurax (en desarrollo)
- [ ] Registros vectoriales SIMD (256/512 bits) para IA
- [ ] Kernel y Sistema Operativo propio

## Compilación y Uso

```bash
# Compilar el ensamblador
gcc src/main.c src/ensamblador.c -o ens

# Ensamblar programa
./ens compilar mi_programa.ensmes

# Ejecutar binario JMA v2
./ens ejecutar mi_programa.jma
```

## Estado del Proyecto

> **Proyecto en desarrollo activo y experimental.**
> 
> - El núcleo funcional (ensamblador + cargador v2) es estable y probado.
> - La ISA es completa pero pueden existir bugs en casos límite.
> - La documentación evoluciona junto con el código.
> 
> *Si encuentras errores o mejoras, abre un Issue o PR.*

## Contribuciones

Busco colaboradores que compartan la visión de tecnología nativa en español y bajo nivel:
- Reportar bugs y casos límite
- Sugerir nuevas instrucciones o mejoras arquitectónicas
- Mejorar documentación y ejemplos
- Ayudar con el lenguaje de alto nivel Neurax

---
*Creado por Joseph | Soberanía Tecnológica en Español* 🇪🇸🧠