Referencia Técnica: Ensamblador Modular Español

1. Mapa de Registros
Para mantener la sintaxis en español, el ensamblador mapea los siguientes nombres a los registros nativos de la arquitectura x86_64:

r_acumulador32 = EAX (32 bits)

r_acumulador64 = RAX (64 bits)

r_base32 = EBX (32 bits)

r_base64 = RBX (64 bits)

r_contador64 = RCX (64 bits)

r_datos64 = RDX (64 bits)

r_puntero64 = RSI (64 bits)

r_destino64 = RDI (64 bits)

r_pila64 = RSP (64 bits)

### Registros Especiales (64 bits)
- `r8_64` = R8 (64 bits)
- `r9_64` = R9 (64 bits)
- `r10_64` = R10 (64 bits)
- `r11_64` = R11 (64 bits)
- `r12_64` = R12 (64 bits)
- `r13_64` = R13 (64 bits)
- `r14_64` = R14 (64 bits)
- `r15_64` = R15 (64 bits)


2. Conjunto de Instrucciones:

1. pausa

Operandos: Ninguno.

Opcode: 0x90

Descripción: No Operation (NOP). No hace nada, útil para temporización o parches.

2. volver

Operandos: Ninguno.

Opcode: 0xC3

Descripción: Return (RET). Devuelve el control a la dirección de retorno.

3. terminar

Operandos: Ninguno.

Opcode: 0xB8 0x3C 0x00 0x00 0x00 (RAX=60) + 0xBF 0x00 0x00 0x00 0x00 (RDI=0) + 0x0F 0x05 (syscall)

Descripción: Realiza una llamada al sistema (sys_exit) para finalizar el programa de forma limpia.

4. copiar (MOV)

Operandos: 
- Inmediato a Registro: copiar <numero>, <registro_destino> (ej. copiar 12345, r_acumulador64)
- Registro a Registro: copiar <registro_origen>, <registro_destino> (ej. copiar r_base64, r_acumulador64)

**Registros soportados:** Todos los registros base (`r_acumulador64`... `r_pila64`) y los especiales (`r8_64`... `r15_64`).

Descripción: Mueve (copia) un valor inmediato o el contenido de un registro origen a un registro destino. No afecta a los flags de la CPU.

--- MODO INMEDIATO (Inmediato a Registro) ---
Escribe un valor numérico directamente en el registro.

Registros de 32 bits (4 bytes):
  0xB8 + imm32 -> MOV EAX, imm32 (Destino: r_acumulador32)
  0xBB + imm32 -> MOV EBX, imm32 (Destino: r_base32)

Registros de 64 bits (8 bytes):
  0x48 0xB8 + imm64 -> MOV RAX, imm64 (Destino: r_acumulador64)
  0x48 0xBB + imm64 -> MOV RBX, imm64 (Destino: r_base64)
  0x48 0xB9 + imm64 -> MOV RCX, imm64 (Destino: r_contador64)
  0x48 0xBA + imm64 -> MOV RDX, imm64 (Destino: r_datos64)
  0x48 0xBE + imm64 -> MOV RSI, imm64 (Destino: r_puntero64)
  0x48 0xBF + imm64 -> MOV RDI, imm64 (Destino: r_destino64)

--- MODO REGISTRO A REGISTRO (64 bits) ---
Copia el valor de un registro a otro. Usa el opcode base 0x48 0x89 (MOV).

Destino: r_acumulador64 (RAX)
  0x48 0x89 0xD8 -> MOV RAX, RBX (origen: r_base64)
  0x48 0x89 0xC8 -> MOV RAX, RCX (origen: r_contador64)
  0x48 0x89 0xD0 -> MOV RAX, RDX (origen: r_datos64)
  0x48 0x89 0xF0 -> MOV RAX, RSI (origen: r_puntero64)
  0x48 0x89 0xF8 -> MOV RAX, RDI (origen: r_destino64)

Destino: r_base64 (RBX)
  0x48 0x89 0xC3 -> MOV RBX, RAX (origen: r_acumulador64)
  0x48 0x89 0xCB -> MOV RBX, RCX (origen: r_contador64)
  0x48 0x89 0xD3 -> MOV RBX, RDX (origen: r_datos64)
  0x48 0x89 0xF3 -> MOV RBX, RSI (origen: r_puntero64)
  0x48 0x89 0xFB -> MOV RBX, RDI (origen: r_destino64)

Destino: r_contador64 (RCX)
  0x48 0x89 0xC1 -> MOV RCX, RAX (origen: r_acumulador64)
  0x48 0x89 0xD9 -> MOV RCX, RBX (origen: r_base64)
  0x48 0x89 0xD1 -> MOV RCX, RDX (origen: r_datos64)
  0x48 0x89 0xF1 -> MOV RCX, RSI (origen: r_puntero64)
  0x48 0x89 0xF9 -> MOV RCX, RDI (origen: r_destino64)

Destino: r_datos64 (RDX)
  0x48 0x89 0xC2 -> MOV RDX, RAX (origen: r_acumulador64)
  0x48 0x89 0xDA -> MOV RDX, RBX (origen: r_base64)
  0x48 0x89 0xCA -> MOV RDX, RCX (origen: r_contador64)
  0x48 0x89 0xF2 -> MOV RDX, RSI (origen: r_puntero64)
  0x48 0x89 0xFA -> MOV RDX, RDI (origen: r_destino64)

Destino: r_puntero64 (RSI)
  0x48 0x89 0xC6 -> MOV RSI, RAX (origen: r_acumulador64)
  0x48 0x89 0xDE -> MOV RSI, RBX (origen: r_base64)
  0x48 0x89 0xCE -> MOV RSI, RCX (origen: r_contador64)
  0x48 0x89 0xD6 -> MOV RSI, RDX (origen: r_datos64)
  0x48 0x89 0xFE -> MOV RSI, RDI (origen: r_destino64)

Destino: r_destino64 (RDI)
  0x48 0x89 0xC7 -> MOV RDI, RAX (origen: r_acumulador64)
  0x48 0x89 0xDF -> MOV RDI, RBX (origen: r_base64)
  0x48 0x89 0xCF -> MOV RDI, RCX (origen: r_contador64)
  0x48 0x89 0xD7 -> MOV RDI, RDX (origen: r_datos64)
  0x48 0x89 0xF7 -> MOV RDI, RSI (origen: r_puntero64)

--- ORIGEN ESPECIAL: r_pila64 (RSP) ---
Copia el valor del puntero de pila (RSP) a otro registro.
  0x48 0x89 0xE0 -> MOV RAX, RSP (Destino: r_acumulador64)
  0x48 0x89 0xE3 -> MOV RBX, RSP (Destino: r_base64)
  0x48 0x89 0xE1 -> MOV RCX, RSP (Destino: r_contador64)
  0x48 0x89 0xE2 -> MOV RDX, RSP (Destino: r_datos64)
  0x48 0x89 0xE6 -> MOV RSI, RSP (Destino: r_puntero64)
  0x48 0x89 0xE7 -> MOV RDI, RSP (Destino: r_destino64)


5. sumar (ADD)

Operandos: 
- Inmediato a Registro: sumar <numero>, <registro_destino> (ej. sumar 10, r_acumulador64)
- Registro a Registro: sumar <registro_origen>, <registro_destino> (ej. sumar r_base64, r_acumulador64)

**Registros soportados:** Todos los registros base (`r_acumulador64`... `r_pila64`) y los especiales (`r8_64`... `r15_64`).

*(Nota técnica: Para los registros r8-r15, el ensamblador utiliza un atajo matemático basado en el prefijo REX y la máscara `& 7` para generar el ModR/M correcto sin necesidad de if/else redundantes).*

Descripción: Suma el valor inmediato o el contenido del registro origen al registro destino, guardando el resultado en el destino. Afecta a los flags de la CPU.

--- MODO INMEDIATO (Inmediato a Registro) ---
Suma un valor numérico (32 bits) directamente al registro.

Registros de 32 bits:
  0x05 + imm32 -> ADD EAX, imm32 (Destino: r_acumulador32)
  0x81 0xC3 + imm32 -> ADD EBX, imm32 (Destino: r_base32)

Registros de 64 bits:
  0x48 0x05 + imm32 -> ADD RAX, imm32 (Destino: r_acumulador64)
  0x48 0x81 0xC3 + imm32 -> ADD RBX, imm32 (Destino: r_base64)
  0x48 0x81 0xC1 + imm32 -> ADD RCX, imm32 (Destino: r_contador64)
  0x48 0x81 0xC2 + imm32 -> ADD RDX, imm32 (Destino: r_datos64)
  0x48 0x81 0xC6 + imm32 -> ADD RSI, imm32 (Destino: r_puntero64)
  0x48 0x81 0xC7 + imm32 -> ADD RDI, imm32 (Destino: r_destino64)
  0x48 0x81 0xC4 + imm32 -> ADD RSP, imm32 (Destino: r_pila64)

--- MODO REGISTRO A REGISTRO (64 bits) ---
Suma el contenido del registro origen al registro destino. Opcode base 0x48 0x01.

Destino: r_acumulador64 (RAX)
  0x48 0x01 0xD8 -> ADD RAX, RBX (origen: r_base64)
  0x48 0x01 0xC0 -> ADD RAX, RAX (origen: r_acumulador64)
  0x48 0x01 0xC8 -> ADD RAX, RCX (origen: r_contador64)
  0x48 0x01 0xD0 -> ADD RAX, RDX (origen: r_datos64)
  0x48 0x01 0xF0 -> ADD RAX, RSI (origen: r_puntero64)
  0x48 0x01 0xF8 -> ADD RAX, RDI (origen: r_destino64)
  0x48 0x01 0xE0 -> ADD RAX, RSP (origen: r_pila64)

Destino: r_base64 (RBX)
  0x48 0x01 0xC3 -> ADD RBX, RAX (origen: r_acumulador64)
  0x48 0x01 0xDB -> ADD RBX, RBX (origen: r_base64)
  0x48 0x01 0xCB -> ADD RBX, RCX (origen: r_contador64)
  0x48 0x01 0xD3 -> ADD RBX, RDX (origen: r_datos64)
  0x48 0x01 0xF3 -> ADD RBX, RSI (origen: r_puntero64)
  0x48 0x01 0xFB -> ADD RBX, RDI (origen: r_destino64)
  0x48 0x01 0xE3 -> ADD RBX, RSP (origen: r_pila64)

Destino: r_contador64 (RCX)
  0x48 0x01 0xC1 -> ADD RCX, RAX (origen: r_acumulador64)
  0x48 0x01 0xD9 -> ADD RCX, RBX (origen: r_base64)
  0x48 0x01 0xC9 -> ADD RCX, RCX (origen: r_contador64)
  0x48 0x01 0xD1 -> ADD RCX, RDX (origen: r_datos64)
  0x48 0x01 0xF1 -> ADD RCX, RSI (origen: r_puntero64)
  0x48 0x01 0xF9 -> ADD RCX, RDI (origen: r_destino64)
  0x48 0x01 0xE1 -> ADD RCX, RSP (origen: r_pila64)

Destino: r_datos64 (RDX)
  0x48 0x01 0xC2 -> ADD RDX, RAX (origen: r_acumulador64)
  0x48 0x01 0xDA -> ADD RDX, RBX (origen: r_base64)
  0x48 0x01 0xCA -> ADD RDX, RCX (origen: r_contador64)
  0x48 0x01 0xD2 -> ADD RDX, RDX (origen: r_datos64)
  0x48 0x01 0xF2 -> ADD RDX, RSI (origen: r_puntero64)
  0x48 0x01 0xFA -> ADD RDX, RDI (origen: r_destino64)
  0x48 0x01 0xE2 -> ADD RDX, RSP (origen: r_pila64)

Destino: r_puntero64 (RSI)
  0x48 0x01 0xC6 -> ADD RSI, RAX (origen: r_acumulador64)
  0x48 0x01 0xDE -> ADD RSI, RBX (origen: r_base64)
  0x48 0x01 0xCE -> ADD RSI, RCX (origen: r_contador64)
  0x48 0x01 0xD6 -> ADD RSI, RDX (origen: r_datos64)
  0x48 0x01 0xF6 -> ADD RSI, RSI (origen: r_puntero64)
  0x48 0x01 0xFE -> ADD RSI, RDI (origen: r_destino64)
  0x48 0x01 0xE6 -> ADD RSI, RSP (origen: r_pila64)

Destino: r_destino64 (RDI)
  0x48 0x01 0xC7 -> ADD RDI, RAX (origen: r_acumulador64)
  0x48 0x01 0xDF -> ADD RDI, RBX (origen: r_base64)
  0x48 0x01 0xCF -> ADD RDI, RCX (origen: r_contador64)
  0x48 0x01 0xD7 -> ADD RDI, RDX (origen: r_datos64)
  0x48 0x01 0xF7 -> ADD RDI, RSI (origen: r_puntero64)
  0x48 0x01 0xFF -> ADD RDI, RDI (origen: r_destino64)
  0x48 0x01 0xE7 -> ADD RDI, RSP (origen: r_pila64)

Destino: r_pila64 (RSP)
  0x48 0x01 0xC4 -> ADD RSP, RAX (origen: r_acumulador64)
  0x48 0x01 0xDC -> ADD RSP, RBX (origen: r_base64)
  0x48 0x01 0xCC -> ADD RSP, RCX (origen: r_contador64)
  0x48 0x01 0xD4 -> ADD RSP, RDX (origen: r_datos64)
  0x48 0x01 0xF4 -> ADD RSP, RSI (origen: r_puntero64)
  0x48 0x01 0xFC -> ADD RSP, RDI (origen: r_destino64)
  0x48 0x01 0xE4 -> ADD RSP, RSP (origen: r_pila64)

6. restar (SUB)

Operandos: 
- Inmediato a Registro: restar <numero>, <registro_destino> (ej. restar 10, r_acumulador64)
- Registro a Registro: restar <registro_origen>, <registro_destino> (ej. restar r_base64, r_acumulador64)

**Registros soportados:** Todos los registros base (`r_acumulador64`... `r_pila64`) y los especiales (`r8_64`... `r15_64`).


Descripción: Resta el valor inmediato o el contenido del registro origen al registro destino, guardando el resultado en el destino. Afecta a los flags de la CPU.

--- MODO INMEDIATO (Inmediato a Registro) ---
Resta un valor numérico (32 bits) directamente al registro.

Registros de 32 bits:
  0x2D + imm32 -> SUB EAX, imm32 (Destino: r_acumulador32)
  0x81 0xEB + imm32 -> SUB EBX, imm32 (Destino: r_base32)

Registros de 64 bits:
  0x48 0x2D + imm32 -> SUB RAX, imm32 (Destino: r_acumulador64)
  0x48 0x81 0xEB + imm32 -> SUB RBX, imm32 (Destino: r_base64)
  0x48 0x81 0xE9 + imm32 -> SUB RCX, imm32 (Destino: r_contador64)
  0x48 0x81 0xEA + imm32 -> SUB RDX, imm32 (Destino: r_datos64)
  0x48 0x81 0xEE + imm32 -> SUB RSI, imm32 (Destino: r_puntero64)
  0x48 0x81 0xEF + imm32 -> SUB RDI, imm32 (Destino: r_destino64)
  0x48 0x81 0xEC + imm32 -> SUB RSP, imm32 (Destino: r_pila64)

--- MODO REGISTRO A REGISTRO (64 bits) ---
Resta el contenido del registro origen al registro destino. Opcode base 0x48 0x29.

Destino: r_acumulador64 (RAX)
  0x48 0x29 0xC0 -> SUB RAX, RAX (origen: r_acumulador64)
  0x48 0x29 0xD8 -> SUB RAX, RBX (origen: r_base64)
  0x48 0x29 0xC8 -> SUB RAX, RCX (origen: r_contador64)
  0x48 0x29 0xD0 -> SUB RAX, RDX (origen: r_datos64)
  0x48 0x29 0xF0 -> SUB RAX, RSI (origen: r_puntero64)
  0x48 0x29 0xF8 -> SUB RAX, RDI (origen: r_destino64)
  0x48 0x29 0xE0 -> SUB RAX, RSP (origen: r_pila64)

Destino: r_base64 (RBX)
  0x48 0x29 0xC3 -> SUB RBX, RAX (origen: r_acumulador64)
  0x48 0x29 0xDB -> SUB RBX, RBX (origen: r_base64)
  0x48 0x29 0xCB -> SUB RBX, RCX (origen: r_contador64)
  0x48 0x29 0xD3 -> SUB RBX, RDX (origen: r_datos64)
  0x48 0x29 0xF3 -> SUB RBX, RSI (origen: r_puntero64)
  0x48 0x29 0xFB -> SUB RBX, RDI (origen: r_destino64)
  0x48 0x29 0xE3 -> SUB RBX, RSP (origen: r_pila64)

Destino: r_contador64 (RCX)
  0x48 0x29 0xC1 -> SUB RCX, RAX (origen: r_acumulador64)
  0x48 0x29 0xD9 -> SUB RCX, RBX (origen: r_base64)
  0x48 0x29 0xC9 -> SUB RCX, RCX (origen: r_contador64)
  0x48 0x29 0xD1 -> SUB RCX, RDX (origen: r_datos64)
  0x48 0x29 0xF1 -> SUB RCX, RSI (origen: r_puntero64)
  0x48 0x29 0xF9 -> SUB RCX, RDI (origen: r_destino64)
  0x48 0x29 0xE1 -> SUB RCX, RSP (origen: r_pila64)

Destino: r_datos64 (RDX)
  0x48 0x29 0xC2 -> SUB RDX, RAX (origen: r_acumulador64)
  0x48 0x29 0xDA -> SUB RDX, RBX (origen: r_base64)
  0x48 0x29 0xCA -> SUB RDX, RCX (origen: r_contador64)
  0x48 0x29 0xD2 -> SUB RDX, RDX (origen: r_datos64)
  0x48 0x29 0xF2 -> SUB RDX, RSI (origen: r_puntero64)
  0x48 0x29 0xFA -> SUB RDX, RDI (origen: r_destino64)
  0x48 0x29 0xE2 -> SUB RDX, RSP (origen: r_pila64)

Destino: r_puntero64 (RSI)
  0x48 0x29 0xC6 -> SUB RSI, RAX (origen: r_acumulador64)
  0x48 0x29 0xDE -> SUB RSI, RBX (origen: r_base64)
  0x48 0x29 0xCE -> SUB RSI, RCX (origen: r_contador64)
  0x48 0x29 0xD6 -> SUB RSI, RDX (origen: r_datos64)
  0x48 0x29 0xF6 -> SUB RSI, RSI (origen: r_puntero64)
  0x48 0x29 0xFE -> SUB RSI, RDI (origen: r_destino64)
  0x48 0x29 0xE6 -> SUB RSI, RSP (origen: r_pila64)

Destino: r_destino64 (RDI)
  0x48 0x29 0xC7 -> SUB RDI, RAX (origen: r_acumulador64)
  0x48 0x29 0xDF -> SUB RDI, RBX (origen: r_base64)
  0x48 0x29 0xCF -> SUB RDI, RCX (origen: r_contador64)
  0x48 0x29 0xD7 -> SUB RDI, RDX (origen: r_datos64)
  0x48 0x29 0xF7 -> SUB RDI, RSI (origen: r_puntero64)
  0x48 0x29 0xFF -> SUB RDI, RDI (origen: r_destino64)
  0x48 0x29 0xE7 -> SUB RDI, RSP (origen: r_pila64)

Destino: r_pila64 (RSP)
  0x48 0x29 0xC4 -> SUB RSP, RAX (origen: r_acumulador64)
  0x48 0x29 0xDC -> SUB RSP, RBX (origen: r_base64)
  0x48 0x29 0xCC -> SUB RSP, RCX (origen: r_contador64)
  0x48 0x29 0xD4 -> SUB RSP, RDX (origen: r_datos64)
  0x48 0x29 0xF4 -> SUB RSP, RSI (origen: r_puntero64)
  0x48 0x29 0xFC -> SUB RSP, RDI (origen: r_destino64)
  0x48 0x29 0xE4 -> SUB RSP, RSP (origen: r_pila64)

7. multiplicar (IMUL)

Operandos: 
- Inmediato a Registro: multiplicar <numero>, <registro_destino> (ej. multiplicar 10, r_acumulador64)
- Registro a Registro: multiplicar <registro_origen>, <registro_destino> (ej. multiplicar r_base64, r_acumulador64)

**Registros soportados:** Todos los registros base (`r_acumulador64`... `r_pila64`) y los especiales (`r8_64`... `r15_64`).


Descripción: Multiplica el valor inmediato (32 bits) o el contenido del registro origen por el registro destino, guardando el resultado en el destino. Afecta a los flags de la CPU (especialmente el de desbordamiento).

--- MODO INMEDIATO (Inmediato a Registro) ---
Multiplica el registro por un valor numérico de 32 bits. Usa el opcode base 0x48 0x69 (IMUL reg, reg, imm32).

Registros de 32 bits:
  0x69 0xC0 + imm32 -> IMUL EAX, EAX, imm32 (Destino: r_acumulador32)
  0x69 0xDB + imm32 -> IMUL EBX, EBX, imm32 (Destino: r_base32)

Registros de 64 bits:
  0x48 0x69 0xC0 + imm32 -> IMUL RAX, RAX, imm32 (Destino: r_acumulador64)
  0x48 0x69 0xDB + imm32 -> IMUL RBX, RBX, imm32 (Destino: r_base64)
  0x48 0x69 0xC9 + imm32 -> IMUL RCX, RCX, imm32 (Destino: r_contador64)
  0x48 0x69 0xD2 + imm32 -> IMUL RDX, RDX, imm32 (Destino: r_datos64)
  0x48 0x69 0xF6 + imm32 -> IMUL RSI, RSI, imm32 (Destino: r_puntero64)
  0x48 0x69 0xFF + imm32 -> IMUL RDI, RDI, imm32 (Destino: r_destino64)
  0x48 0x69 0xE4 + imm32 -> IMUL RSP, RSP, imm32 (Destino: r_pila64)

--- MODO REGISTRO A REGISTRO (64 bits) ---
Multiplica el contenido del registro origen por el registro destino. Opcode base 0x48 0x0F 0xAF.

Destino: r_acumulador64 (RAX)
  0x48 0x0F 0xAF 0xC3 -> RAX * RBX (origen: r_base64)
  0x48 0x0F 0xAF 0xC0 -> RAX * RAX (origen: r_acumulador64)
  0x48 0x0F 0xAF 0xC1 -> RAX * RCX (origen: r_contador64)
  0x48 0x0F 0xAF 0xC2 -> RAX * RDX (origen: r_datos64)
  0x48 0x0F 0xAF 0xC6 -> RAX * RSI (origen: r_puntero64)
  0x48 0x0F 0xAF 0xC7 -> RAX * RDI (origen: r_destino64)
  0x48 0x0F 0xAF 0xC4 -> RAX * RSP (origen: r_pila64)

Destino: r_base64 (RBX)
  0x48 0x0F 0xAF 0xD8 -> RBX * RAX (origen: r_acumulador64)
  0x48 0x0F 0xAF 0xDB -> RBX * RBX (origen: r_base64)
  0x48 0x0F 0xAF 0xD9 -> RBX * RCX (origen: r_contador64)
  0x48 0x0F 0xAF 0xDA -> RBX * RDX (origen: r_datos64)
  0x48 0x0F 0xAF 0xDE -> RBX * RSI (origen: r_puntero64)
  0x48 0x0F 0xAF 0xDF -> RBX * RDI (origen: r_destino64)
  0x48 0x0F 0xAF 0xDC -> RBX * RSP (origen: r_pila64)

Destino: r_contador64 (RCX)
  0x48 0x0F 0xAF 0xC8 -> RCX * RAX (origen: r_acumulador64)
  0x48 0x0F 0xAF 0xCB -> RCX * RBX (origen: r_base64)
  0x48 0x0F 0xAF 0xC9 -> RCX * RCX (origen: r_contador64)
  0x48 0x0F 0xAF 0xCA -> RCX * RDX (origen: r_datos64)
  0x48 0x0F 0xAF 0xCE -> RCX * RSI (origen: r_puntero64)
  0x48 0x0F 0xAF 0xCF -> RCX * RDI (origen: r_destino64)
  0x48 0x0F 0xAF 0xCC -> RCX * RSP (origen: r_pila64)

Destino: r_datos64 (RDX)
  0x48 0x0F 0xAF 0xD0 -> RDX * RAX (origen: r_acumulador64)
  0x48 0x0F 0xAF 0xD3 -> RDX * RBX (origen: r_base64)
  0x48 0x0F 0xAF 0xD1 -> RDX * RCX (origen: r_contador64)
  0x48 0x0F 0xAF 0xD2 -> RDX * RDX (origen: r_datos64)
  0x48 0x0F 0xAF 0xD6 -> RDX * RSI (origen: r_puntero64)
  0x48 0x0F 0xAF 0xD7 -> RDX * RDI (origen: r_destino64)
  0x48 0x0F 0xAF 0xD4 -> RDX * RSP (origen: r_pila64)

Destino: r_puntero64 (RSI)
  0x48 0x0F 0xAF 0xF0 -> RSI * RAX (origen: r_acumulador64)
  0x48 0x0F 0xAF 0xF3 -> RSI * RBX (origen: r_base64)
  0x48 0x0F 0xAF 0xF1 -> RSI * RCX (origen: r_contador64)
  0x48 0x0F 0xAF 0xF2 -> RSI * RDX (origen: r_datos64)
  0x48 0x0F 0xAF 0xF6 -> RSI * RSI (origen: r_puntero64)
  0x48 0x0F 0xAF 0xF7 -> RSI * RDI (origen: r_destino64)
  0x48 0x0F 0xAF 0xF4 -> RSI * RSP (origen: r_pila64)

Destino: r_destino64 (RDI)
  0x48 0x0F 0xAF 0xF8 -> RDI * RAX (origen: r_acumulador64)
  0x48 0x0F 0xAF 0xFB -> RDI * RBX (origen: r_base64)
  0x48 0x0F 0xAF 0xF9 -> RDI * RCX (origen: r_contador64)
  0x48 0x0F 0xAF 0xFA -> RDI * RDX (origen: r_datos64)
  0x48 0x0F 0xAF 0xFE -> RDI * RSI (origen: r_puntero64)
  0x48 0x0F 0xAF 0xFF -> RDI * RDI (origen: r_destino64)
  0x48 0x0F 0xAF 0xFC -> RDI * RSP (origen: r_pila64)

Destino: r_pila64 (RSP)
  0x48 0x0F 0xAF 0xE0 -> RSP * RAX (origen: r_acumulador64)
  0x48 0x0F 0xAF 0xE3 -> RSP * RBX (origen: r_base64)
  0x48 0x0F 0xAF 0xE1 -> RSP * RCX (origen: r_contador64)
  0x48 0x0F 0xAF 0xE2 -> RSP * RDX (origen: r_datos64)
  0x48 0x0F 0xAF 0xE6 -> RSP * RSI (origen: r_puntero64)
  0x48 0x0F 0xAF 0xE7 -> RSP * RDI (origen: r_destino64)
  0x48 0x0F 0xAF 0xE4 -> RSP * RSP (origen: r_pila64)

8. Etiquetas (Definición)

Operandos: Ninguno.

Sintaxis: nombre_etiqueta:

Descripción: Marca una posición específica en el código para que pueda ser referenciada por instrucciones de salto. No genera ningún byte en el archivo binario, es solo una referencia de memoria.

9. saltar (JMP Relativo)

Operandos: Etiqueta (ej. saltar inicio).

Opcode: 0xE9 + int32 (Distancia relativa en bytes).

Descripción: Salta incondicionalmente a la posición de la etiqueta especificada. El ensamblador calcula la distancia en bytes desde el final de la instrucción hasta la etiqueta.
Soporte bidireccional (Backpatching):

Saltos hacia atrás: Se calculan en el momento (la etiqueta ya ha sido leída).

Saltos hacia adelante: Si la etiqueta aún no ha sido leída, el ensamblador escribe un hueco temporal (E9 00 00 00 00), guarda la posición en una lista de pendientes y, al encontrar la etiqueta más adelante, usa fseek para volver al hueco y rellenar la distancia real.

10. comparar (CMP)

Operandos: 
- Inmediato a Registro: comparar <numero>, <registro_destino> (ej. comparar 10, r_acumulador64)
- Registro a Registro: comparar <registro_origen>, <registro_destino> (ej. comparar r_base64, r_acumulador64)

**Registros soportados:** Todos los registros base (`r_acumulador64`... `r_pila64`) y los especiales (`r8_64`... `r15_64`).

Descripción: Resta el operando origen al operando destino, pero **no guarda el resultado**. Su única función es actualizar los flags de la CPU (como Zero Flag, Sign Flag, etc.) para que una instrucción de salto condicional posterior (como `saltar_si_igual` o `saltar_si_mayor`) pueda decidir si saltar o no.

--- MODO INMEDIATO (Inmediato a Registro) ---
Compara el registro con un valor numérico (32 bits).

Registros de 32 bits:
  0x3D + imm32 -> CMP EAX, imm32 (Destino: r_acumulador32)
  0x81 0xFB + imm32 -> CMP EBX, imm32 (Destino: r_base32)

Registros de 64 bits:
  0x48 0x3D + imm32 -> CMP RAX, imm32 (Destino: r_acumulador64)
  0x48 0x81 0xFB + imm32 -> CMP RBX, imm32 (Destino: r_base64)
  0x48 0x81 0xF9 + imm32 -> CMP RCX, imm32 (Destino: r_contador64)
  0x48 0x81 0xFA + imm32 -> CMP RDX, imm32 (Destino: r_datos64)
  0x48 0x81 0xFE + imm32 -> CMP RSI, imm32 (Destino: r_puntero64)
  0x48 0x81 0xFF + imm32 -> CMP RDI, imm32 (Destino: r_destino64)
  0x48 0x81 0xFC + imm32 -> CMP RSP, imm32 (Destino: r_pila64)

--- MODO REGISTRO A REGISTRO (64 bits) ---
Compara el contenido del registro origen con el registro destino. Opcode base 0x48 0x39.

Destino: r_acumulador64 (RAX)
  0x48 0x39 0xD8 -> CMP RAX, RBX (origen: r_base64)
  0x48 0x39 0xC0 -> CMP RAX, RAX (origen: r_acumulador64)
  0x48 0x39 0xC8 -> CMP RAX, RCX (origen: r_contador64)
  0x48 0x39 0xD0 -> CMP RAX, RDX (origen: r_datos64)
  0x48 0x39 0xF0 -> CMP RAX, RSI (origen: r_puntero64)
  0x48 0x39 0xF8 -> CMP RAX, RDI (origen: r_destino64)
  0x48 0x39 0xE0 -> CMP RAX, RSP (origen: r_pila64)

Destino: r_base64 (RBX)
  0x48 0x39 0xC3 -> CMP RBX, RAX (origen: r_acumulador64)
  0x48 0x39 0xDB -> CMP RBX, RBX (origen: r_base64)
  0x48 0x39 0xCB -> CMP RBX, RCX (origen: r_contador64)
  0x48 0x39 0xD3 -> CMP RBX, RDX (origen: r_datos64)
  0x48 0x39 0xF3 -> CMP RBX, RSI (origen: r_puntero64)
  0x48 0x39 0xFB -> CMP RBX, RDI (origen: r_destino64)
  0x48 0x39 0xE3 -> CMP RBX, RSP (origen: r_pila64)

Destino: r_contador64 (RCX)
  0x48 0x39 0xC1 -> CMP RCX, RAX (origen: r_acumulador64)
  0x48 0x39 0xD9 -> CMP RCX, RBX (origen: r_base64)
  0x48 0x39 0xC9 -> CMP RCX, RCX (origen: r_contador64)
  0x48 0x39 0xD1 -> CMP RCX, RDX (origen: r_datos64)
  0x48 0x39 0xF1 -> CMP RCX, RSI (origen: r_puntero64)
  0x48 0x39 0xF9 -> CMP RCX, RDI (origen: r_destino64)
  0x48 0x39 0xE1 -> CMP RCX, RSP (origen: r_pila64)

Destino: r_datos64 (RDX)
  0x48 0x39 0xC2 -> CMP RDX, RAX (origen: r_acumulador64)
  0x48 0x39 0xDA -> CMP RDX, RBX (origen: r_base64)
  0x48 0x39 0xCA -> CMP RDX, RCX (origen: r_contador64)
  0x48 0x39 0xD2 -> CMP RDX, RDX (origen: r_datos64)
  0x48 0x39 0xF2 -> CMP RDX, RSI (origen: r_puntero64)
  0x48 0x39 0xFA -> CMP RDX, RDI (origen: r_destino64)
  0x48 0x39 0xE2 -> CMP RDX, RSP (origen: r_pila64)

Destino: r_puntero64 (RSI)
  0x48 0x39 0xC6 -> CMP RSI, RAX (origen: r_acumulador64)
  0x48 0x39 0xDE -> CMP RSI, RBX (origen: r_base64)
  0x48 0x39 0xCE -> CMP RSI, RCX (origen: r_contador64)
  0x48 0x39 0xD6 -> CMP RSI, RDX (origen: r_datos64)
  0x48 0x39 0xF6 -> CMP RSI, RSI (origen: r_puntero64)
  0x48 0x39 0xFE -> CMP RSI, RDI (origen: r_destino64)
  0x48 0x39 0xE6 -> CMP RSI, RSP (origen: r_pila64)

Destino: r_destino64 (RDI)
  0x48 0x39 0xC7 -> CMP RDI, RAX (origen: r_acumulador64)
  0x48 0x39 0xDF -> CMP RDI, RBX (origen: r_base64)
  0x48 0x39 0xCF -> CMP RDI, RCX (origen: r_contador64)
  0x48 0x39 0xD7 -> CMP RDI, RDX (origen: r_datos64)
  0x48 0x39 0xF7 -> CMP RDI, RSI (origen: r_puntero64)
  0x48 0x39 0xFF -> CMP RDI, RDI (origen: r_destino64)
  0x48 0x39 0xE7 -> CMP RDI, RSP (origen: r_pila64)

Destino: r_pila64 (RSP)
  0x48 0x39 0xC4 -> CMP RSP, RAX (origen: r_acumulador64)
  0x48 0x39 0xDC -> CMP RSP, RBX (origen: r_base64)
  0x48 0x39 0xCC -> CMP RSP, RCX (origen: r_contador64)
  0x48 0x39 0xD4 -> CMP RSP, RDX (origen: r_datos64)
  0x48 0x39 0xF4 -> CMP RSP, RSI (origen: r_puntero64)
  0x48 0x39 0xFC -> CMP RSP, RDI (origen: r_destino64)
  0x48 0x39 0xE4 -> CMP RSP, RSP (origen: r_pila64)

11. saltar_si_igual (JE)
Operandos: Etiqueta (ej. saltar_si_igual fin).
Opcode: 0x0F 0x84 + int32 (Distancia relativa de 32 bits).
Descripción: Salta a la etiqueta si la última comparación resultó en igualdad (Zero Flag = 1). Soporta backpatching (saltos hacia adelante y hacia atrás).
Secuencia de bytes generada: 0x0F 0x84 + int32 (distancia calculada).


12. saltar_si_distinto (JNE)
Operandos: Etiqueta (ej. saltar_si_distinto bucle).
Opcode: 0x0F 0x85 + int32 (Distancia relativa de 32 bits).
Descripción: Salta a la etiqueta si la última comparación resultó en desigualdad (Zero Flag = 0). Soporta backpatching.
Secuencia de bytes generada: 0x0F 0x85 + int32 (distancia calculada).

13. llamar (CALL)

Operandos: Etiqueta (ej. llamar mi_funcion).

Opcode: 0xE8 + int32 (Distancia relativa en bytes).

Descripción: Llama a una subrutina. Antes de saltar, la CPU guarda automáticamente la dirección de la siguiente instrucción en la pila (para saber a dónde volver). Se usa junto con volver (RET) para crear funciones. Soporta backpatching.

14. .codigo (Sección de Código)

Descripción: Indica que las siguientes líneas contienen instrucciones ejecutables. Es la sección por defecto.

15. .datos (Sección de Datos)

Descripción: Indica que las siguientes líneas contienen datos inicializados (variables, strings, etc.). Estos datos se escriben al final del archivo binario.

16. Directivas de definicion de datos

definir_byte (1 byte): Escribe un valor de 8 bits.

definir_palabra (2 bytes): Escribe un valor de 16 bits (little-endian).

definir_doble (4 bytes): Escribe un valor de 32 bits (little-endian).

definir_cuadruple (8 bytes): Escribe un valor de 64 bits (little-endian).

17. Directivas de reserva de datos

reservar_byte (1 bytes): Reserva N bytes en cero.

reservar_palabra (2 bytes): Reserva N palabras (N*2 bytes) en cero.

reservar_doble (4 bytes): Reserva N dobles palabras (N*4 bytes) en cero.

reservar_cuadruple (8 bytes): Reserva N cuádruples palabras (N*8 bytes) en cero.


18. cargar_direccion (LEA)

Operandos: Etiqueta, Registro (ej. cargar_direccion mensaje, r_base64).

Descripción: Carga la dirección de memoria relativa (RIP-relative) de una etiqueta en un registro. Se usa para obtener punteros a strings o buffers, ya que no copia el contenido de la etiqueta, sino su posición en memoria.

Opcode base 0x48 0x8D:

0x48 0x8D 0x05 + int32 -> LEA RAX, [RIP + disp32] (Destino: r_acumulador64)

0x48 0x8D 0x1D + int32 -> LEA RBX, [RIP + disp32] (Destino: r_base64)

Descripción de la distancia: El valor int32 es la distancia en bytes desde el final de la instrucción hasta la etiqueta. Soporta backpatching si la etiqueta está más abajo en el código.


19. dividir (DIV)

Operandos: Registro (ej. dividir r_datos64).

Descripción: Divide el valor de RDX:RAX entre el registro especificado. La instrucción limpia RDX a cero antes de dividir para que el dividendo sea solo el valor de RAX. El cociente se guarda en RAX y el resto en RDX.

**Registros soportados:** Todos los registros base (`r_acumulador64`... `r_pila64`) y los especiales (`r8_64`... `r15_64`).

Secuencia de bytes generada (ej. dividir r_datos64):

48 31 D2 -> XOR RDX, RDX (Limpia el resto).

48 F7 F2 -> DIV RDX (Divide RAX entre RDX).

Registros soportados como divisor:

r_acumulador64 -> 48 F7 F0 (DIV RAX)

r_base64 -> 48 F7 F3 (DIV RBX)

r_contador64 -> 48 F7 F1 (DIV RCX)

r_datos64 -> 48 F7 F2 (DIV RDX)

r_puntero64 -> 48 F7 F6 (DIV RSI)

r_destino64 -> 48 F7 F7 (DIV RDI)

20. o_exclusivo (XOR)

Operandos: Registro_origen, Registro_destino (ej. o_exclusivo r_base64, r_acumulador64).

Descripción: Realiza la operación "O exclusiva" (XOR) bit a bit entre el registro origen y el registro destino, guardando el resultado en el destino. 
Nota importante: No acepta valores inmediatos (números), solo registros. Si usas el mismo registro como origen y destino (ej. o_exclusivo r_acumulador64, r_acumulador64), el registro se limpiará a cero (se pone a 0), lo cual es muy útil para inicializar variables.

Opcode base 0x48 0x31 (XOR de 64 bits):

Destino: r_acumulador64 (RAX)
  0x48 0x31 0xD8 -> XOR RAX, RBX (origen: r_base64)
  0x48 0x31 0xC0 -> XOR RAX, RAX (origen: r_acumulador64 - limpiar)
  0x48 0x31 0xC8 -> XOR RAX, RCX (origen: r_contador64)
  0x48 0x31 0xD0 -> XOR RAX, RDX (origen: r_datos64)
  0x48 0x31 0xF0 -> XOR RAX, RSI (origen: r_puntero64)
  0x48 0x31 0xF8 -> XOR RAX, RDI (origen: r_destino64)

Destino: r_base64 (RBX)
  0x48 0x31 0xC3 -> XOR RBX, RAX (origen: r_acumulador64)
  0x48 0x31 0xDB -> XOR RBX, RBX (origen: r_base64 - limpiar)
  0x48 0x31 0xCB -> XOR RBX, RCX (origen: r_contador64)
  0x48 0x31 0xD3 -> XOR RBX, RDX (origen: r_datos64)
  0x48 0x31 0xF3 -> XOR RBX, RSI (origen: r_puntero64)
  0x48 0x31 0xFB -> XOR RBX, RDI (origen: r_destino64)

Destino: r_contador64 (RCX)
  0x48 0x31 0xC1 -> XOR RCX, RAX (origen: r_acumulador64)
  0x48 0x31 0xD9 -> XOR RCX, RBX (origen: r_base64)
  0x48 0x31 0xC9 -> XOR RCX, RCX (origen: r_contador64 - limpiar)
  0x48 0x31 0xD1 -> XOR RCX, RDX (origen: r_datos64)
  0x48 0x31 0xF1 -> XOR RCX, RSI (origen: r_puntero64)
  0x48 0x31 0xF9 -> XOR RCX, RDI (origen: r_destino64)

Destino: r_datos64 (RDX)
  0x48 0x31 0xC2 -> XOR RDX, RAX (origen: r_acumulador64)
  0x48 0x31 0xDA -> XOR RDX, RBX (origen: r_base64)
  0x48 0x31 0xCA -> XOR RDX, RCX (origen: r_contador64)
  0x48 0x31 0xD2 -> XOR RDX, RDX (origen: r_datos64 - limpiar)
  0x48 0x31 0xF2 -> XOR RDX, RSI (origen: r_puntero64)
  0x48 0x31 0xFA -> XOR RDX, RDI (origen: r_destino64)

Destino: r_puntero64 (RSI)
  0x48 0x31 0xC6 -> XOR RSI, RAX (origen: r_acumulador64)
  0x48 0x31 0xDE -> XOR RSI, RBX (origen: r_base64)
  0x48 0x31 0xCE -> XOR RSI, RCX (origen: r_contador64)
  0x48 0x31 0xD6 -> XOR RSI, RDX (origen: r_datos64)
  0x48 0x31 0xF6 -> XOR RSI, RSI (origen: r_puntero64 - limpiar)
  0x48 0x31 0xFE -> XOR RSI, RDI (origen: r_destino64)

Destino: r_destino64 (RDI)
  0x48 0x31 0xC7 -> XOR RDI, RAX (origen: r_acumulador64)
  0x48 0x31 0xDF -> XOR RDI, RBX (origen: r_base64)
  0x48 0x31 0xCF -> XOR RDI, RCX (origen: r_contador64)
  0x48 0x31 0xD7 -> XOR RDI, RDX (origen: r_datos64)
  0x48 0x31 0xF7 -> XOR RDI, RSI (origen: r_puntero64)
  0x48 0x31 0xFF -> XOR RDI, RDI (origen: r_destino64 - limpiar)

Destino: r_pila64 (RSP)
  0x48 0x31 0xC4 -> XOR RSP, RAX (origen: r_acumulador64)
  0x48 0x31 0xDC -> XOR RSP, RBX (origen: r_base64)
  0x48 0x31 0xCC -> XOR RSP, RCX (origen: r_contador64)
  0x48 0x31 0xD4 -> XOR RSP, RDX (origen: r_datos64)
  0x48 0x31 0xF4 -> XOR RSP, RSI (origen: r_puntero64)
  0x48 0x31 0xFC -> XOR RSP, RDI (origen: r_destino64)
  0x48 0x31 0xE4 -> XOR RSP, RSP (origen: r_pila64 - limpiar)

21. guardar_puntero (MOV [reg + offset], reg)

Operandos: Registro_origen, [Registro_direccion + offset] (ej. guardar_puntero r_acumulador64, [r_base64+8]).

Descripción: Guarda el contenido de un registro de 64 bits en la dirección de memoria apuntada por otro registro, permitiendo sumar o restar un desplazamiento (offset). Es fundamental para escribir en arrays o estructuras de datos.
El ensamblador detecta el signo '+' o '-' y calcula el desplazamiento (disp32). Para activar este modo, suma 0x80 al byte ModR/M base.

Opcode base 0x48 0x89 (MOV de 64 bits):

[Sin offset: MOV [RDI], RAX -> 0x48 0x89 0x07]
[Con offset: MOV [RDI + 8], RAX -> 0x48 0x89 0x87 0x08 0x00 0x00 0x00]

[ADVERTENCIA CRÍTICA]
Esta instrucción escribe 8 bytes (64 bits). Si se usa para escribir caracteres (bytes) en un buffer, los 7 bytes posteriores se sobrescribirán con ceros. Usar 'guardar_byte_puntero' para texto.

22. guardar_byte_puntero (MOV BYTE PTR [reg + offset], reg8)

Operandos: Registro_origen, [Registro_direccion + offset] (ej. guardar_byte_puntero r_acumulador64, [r_base64+1]).

Descripción: Guarda únicamente el byte menos significativo (8 bits) del registro de origen en la dirección de memoria especificada, con soporte completo para desplazamientos (offsets). 
Es la instrucción definitiva para construir strings carácter a carácter sin corromper los bytes adyacentes.

Opcode base 0x88 (MOV r/m8, r8):
Ejemplo sin offset: guardar_byte_puntero r_acumulador64, [r_base64]
  -> 0x88 0x03
Ejemplo con offset: guardar_byte_puntero r_acumulador64, [r_base64+1]
  -> 0x88 0x43 0x01 0x00 0x00 0x00

[Nota]: Esta instrucción soporta la matriz completa de 7x7 registros base y registros origen, más la lógica de offset (ModR/M | 0x80).

23. saltar_si_mayor (JG)

Operandos: Etiqueta (ej. saltar_si_mayor fin).

Opcode: 0x0F 0x8F + int32 (Distancia relativa de 32 bits).

Descripción: Salta a la etiqueta si la última comparación resultó en que el operando destino es estrictamente mayor que el origen (usando comparación con signo). Soporta backpatching.

24. saltar_si_menor (JL)

Operandos: Etiqueta (ej. saltar_si_menor bucle).

Opcode: 0x0F 0x8C + int32 (Distancia relativa de 32 bits).

Descripción: Salta a la etiqueta si la última comparación resultó en que el operando destino es estrictamente menor que el origen. Soporta backpatching.

25. saltar_si_mayor_o_igual (JGE)

Operandos: Etiqueta.

Opcode: 0x0F 0x8D + int32.

Descripción: Salta a la etiqueta si el destino es mayor o igual que el origen. Soporta backpatching.

26. saltar_si_menor_o_igual (JLE)

Operandos: Etiqueta.

Opcode: 0x0F 0x8E + int32.

Descripción: Salta a la etiqueta si el destino es menor o igual que el origen. Soporta backpatching.

27. guardar_indirecto (MOV [reg], reg)

Operandos: Registro_origen, [Registro_direccion] (ej. guardar_indirecto r_acumulador64, [r_puntero64]).

Descripción: Guarda el contenido de un registro de 64 bits en la dirección de memoria apuntada por otro registro. 
Nota: Esta instrucción es funcionalmente idéntica a guardar_puntero, pero se ofrece con un nombre alternativo para mejorar la legibilidad del código fuente en determinados contextos (por ejemplo, al escribir a través de punteros dobles o al manejar estructuras de datos).

Opcode base 0x48 0x89 (MOV de 64 bits):
Escribe 8 bytes (64 bits) en memoria.

Combinaciones soportadas (Registro de dirección -> Registro de origen):

Si la dirección está en r_puntero64 (RSI):
  0x48 0x89 0x06 -> MOV [RSI], RAX
  0x48 0x89 0x1E -> MOV [RSI], RBX
  0x48 0x89 0x0E -> MOV [RSI], RCX
  0x48 0x89 0x16 -> MOV [RSI], RDX
  0x48 0x89 0x36 -> MOV [RSI], RSI
  0x48 0x89 0x3E -> MOV [RSI], RDI
  0x48 0x89 0x26 -> MOV [RSI], RSP

Si la dirección está en r_destino64 (RDI):
  0x48 0x89 0x07 -> MOV [RDI], RAX
  0x48 0x89 0x1F -> MOV [RDI], RBX
  0x48 0x89 0x0F -> MOV [RDI], RCX
  0x48 0x89 0x17 -> MOV [RDI], RDX
  0x48 0x89 0x37 -> MOV [RDI], RSI
  0x48 0x89 0x3F -> MOV [RDI], RDI
  0x48 0x89 0x27 -> MOV [RDI], RSP

Si la dirección está en r_acumulador64 (RAX):
  0x48 0x89 0x00 -> MOV [RAX], RAX
  0x48 0x89 0x18 -> MOV [RAX], RBX
  0x48 0x89 0x08 -> MOV [RAX], RCX
  0x48 0x89 0x10 -> MOV [RAX], RDX
  0x48 0x89 0x30 -> MOV [RAX], RSI
  0x48 0x89 0x38 -> MOV [RAX], RDI
  0x48 0x89 0x20 -> MOV [RAX], RSP

Si la dirección está en r_base64 (RBX):
  0x48 0x89 0x03 -> MOV [RBX], RAX
  0x48 0x89 0x1B -> MOV [RBX], RBX
  0x48 0x89 0x0B -> MOV [RBX], RCX
  0x48 0x89 0x13 -> MOV [RBX], RDX
  0x48 0x89 0x33 -> MOV [RBX], RSI
  0x48 0x89 0x3B -> MOV [RBX], RDI
  0x48 0x89 0x23 -> MOV [RBX], RSP

Si la dirección está en r_contador64 (RCX):
  0x48 0x89 0x01 -> MOV [RCX], RAX
  0x48 0x89 0x19 -> MOV [RCX], RBX
  0x48 0x89 0x09 -> MOV [RCX], RCX
  0x48 0x89 0x11 -> MOV [RCX], RDX
  0x48 0x89 0x31 -> MOV [RCX], RSI
  0x48 0x89 0x39 -> MOV [RCX], RDI
  0x48 0x89 0x21 -> MOV [RCX], RSP

Si la dirección está en r_datos64 (RDX):
  0x48 0x89 0x02 -> MOV [RDX], RAX
  0x48 0x89 0x1A -> MOV [RDX], RBX
  0x48 0x89 0x0A -> MOV [RDX], RCX
  0x48 0x89 0x12 -> MOV [RDX], RDX
  0x48 0x89 0x32 -> MOV [RDX], RSI
  0x48 0x89 0x3A -> MOV [RDX], RDI
  0x48 0x89 0x22 -> MOV [RDX], RSP

Si la dirección está en r_pila64 (RSP):
  0x48 0x89 0x04 -> MOV [RSP], RAX
  0x48 0x89 0x1C -> MOV [RSP], RBX
  0x48 0x89 0x0C -> MOV [RSP], RCX
  0x48 0x89 0x14 -> MOV [RSP], RDX
  0x48 0x89 0x34 -> MOV [RSP], RSI
  0x48 0x89 0x3C -> MOV [RSP], RDI
  0x48 0x89 0x24 -> MOV [RSP], RSP

[ADVERTENCIA]: Al igual que guardar_puntero, esta instrucción escribe 64 bits (8 bytes). Si se usa con buffers de bytes (como strings), sobrescribirá los 7 bytes siguientes a la dirección. Para escribir un solo byte, usar 'guardar_byte_puntero'.

28. desplazar_izquierda (SHL)

Operandos: 
- Inmediato a Registro: desplazar_izquierda <numero>, <registro_destino> (ej. desplazar_izquierda 3, r_acumulador64)
- Registro a Registro: desplazar_izquierda r_contador8, <registro_destino> (ej. desplazar_izquierda r_contador8, r_base64)

Descripción: Desplaza los bits del registro destino hacia la izquierda. Esto equivale a multiplicar el valor por potencias de 2. 
Nota sobre el origen: La arquitectura x86_64 exige que el registro que indica la cantidad de desplazamiento sea obligatoriamente 'r_contador8' (CL, los 8 bits bajos de RCX).

MODO INMEDIATO:
Desplaza por una cantidad fija (inmediato de 8 bits):
  0x48 0xC1 0xE0 + imm8 -> SHL RAX, imm8 (Destino: r_acumulador64)
  0x48 0xC1 0xE3 + imm8 -> SHL RBX, imm8 (Destino: r_base64)
  0x48 0xC1 0xE1 + imm8 -> SHL RCX, imm8 (Destino: r_contador64)
  0x48 0xC1 0xE2 + imm8 -> SHL RDX, imm8 (Destino: r_datos64)
  0x48 0xC1 0xE6 + imm8 -> SHL RSI, imm8 (Destino: r_puntero64)
  0x48 0xC1 0xE7 + imm8 -> SHL RDI, imm8 (Destino: r_destino64)
  0x48 0xC1 0xE4 + imm8 -> SHL RSP, imm8 (Destino: r_pila64)

MODO REGISTRO A REGISTRO (El origen SIEMPRE es r_contador8 / CL):
  0x48 0xD3 0xE0 -> SHL RAX, CL (Destino: r_acumulador64)
  0x48 0xD3 0xE3 -> SHL RBX, CL (Destino: r_base64)
  0x48 0xD3 0xE1 -> SHL RCX, CL (Destino: r_contador64)
  0x48 0xD3 0xE2 -> SHL RDX, CL (Destino: r_datos64)
  0x48 0xD3 0xE6 -> SHL RSI, CL (Destino: r_puntero64)
  0x48 0xD3 0xE7 -> SHL RDI, CL (Destino: r_destino64)
  0x48 0xD3 0xE4 -> SHL RSP, CL (Destino: r_pila64)


29. desplazar_derecha (SHR)

Operandos: 
- Inmediato a Registro: desplazar_derecha <numero>, <registro_destino> (ej. desplazar_derecha 2, r_acumulador64)
- Registro a Registro: desplazar_derecha r_contador8, <registro_destino> (ej. desplazar_derecha r_contador8, r_base64)

Descripción: Desplaza los bits del registro destino hacia la derecha. Esto equivale a dividir el valor por potencias de 2 (de forma entera y sin signo). 
Nota sobre el origen: Al igual que con el desplazamiento a la izquierda, el hardware obliga a que el registro contador sea 'r_contador8' (CL).

MODO INMEDIATO:
Desplaza por una cantidad fija (inmediato de 8 bits):
  0x48 0xC1 0xE8 + imm8 -> SHR RAX, imm8 (Destino: r_acumulador64)
  0x48 0xC1 0xEB + imm8 -> SHR RBX, imm8 (Destino: r_base64)
  0x48 0xC1 0xE9 + imm8 -> SHR RCX, imm8 (Destino: r_contador64)
  0x48 0xC1 0xEA + imm8 -> SHR RDX, imm8 (Destino: r_datos64)
  0x48 0xC1 0xEE + imm8 -> SHR RSI, imm8 (Destino: r_puntero64)
  0x48 0xC1 0xEF + imm8 -> SHR RDI, imm8 (Destino: r_destino64)
  0x48 0xC1 0xEC + imm8 -> SHR RSP, imm8 (Destino: r_pila64)

MODO REGISTRO A REGISTRO (El origen SIEMPRE es r_contador8 / CL):
  0x48 0xD3 0xE8 -> SHR RAX, CL (Destino: r_acumulador64)
  0x48 0xD3 0xEB -> SHR RBX, CL (Destino: r_base64)
  0x48 0xD3 0xE9 -> SHR RCX, CL (Destino: r_contador64)
  0x48 0xD3 0xEA -> SHR RDX, CL (Destino: r_datos64)
  0x48 0xD3 0xEE -> SHR RSI, CL (Destino: r_puntero64)
  0x48 0xD3 0xEF -> SHR RDI, CL (Destino: r_destino64)
  0x48 0xD3 0xEC -> SHR RSP, CL (Destino: r_pila64)

30. y (AND - Y Lógico)

Operandos: 
- Inmediato a Registro: y <numero>, <registro_destino> (ej. y 255, r_acumulador64)
- Registro a Registro: y <registro_origen>, <registro_destino> (ej. y r_base64, r_acumulador64)

Descripción: Realiza la operación lógica AND (Y) bit a bit entre el origen y el destino, guardando el resultado en el destino. Se usa comúnmente para aplicar máscaras de bits (por ejemplo, para aislar los bits menos significativos).

--- MODO INMEDIATO (Inmediato a Registro) ---
Realiza la operación AND con un valor numérico (32 bits) directamente al registro.

Registros de 64 bits:
  0x48 0x25 + imm32 -> AND RAX, imm32 (Destino: r_acumulador64)
  0x48 0x81 0xE3 + imm32 -> AND RBX, imm32 (Destino: r_base64)
  0x48 0x81 0xE1 + imm32 -> AND RCX, imm32 (Destino: r_contador64)
  0x48 0x81 0xE2 + imm32 -> AND RDX, imm32 (Destino: r_datos64)
  0x48 0x81 0xE6 + imm32 -> AND RSI, imm32 (Destino: r_puntero64)
  0x48 0x81 0xE7 + imm32 -> AND RDI, imm32 (Destino: r_destino64)
  0x48 0x81 0xE4 + imm32 -> AND RSP, imm32 (Destino: r_pila64)

--- MODO REGISTRO A REGISTRO (64 bits) ---
Opcode base 0x48 0x21.

Destino: r_acumulador64 (RAX)
  0x48 0x21 0xD8 -> AND RAX, RBX (origen: r_base64)
  0x48 0x21 0xC0 -> AND RAX, RAX (origen: r_acumulador64)
  0x48 0x21 0xC8 -> AND RAX, RCX (origen: r_contador64)
  0x48 0x21 0xD0 -> AND RAX, RDX (origen: r_datos64)
  0x48 0x21 0xF0 -> AND RAX, RSI (origen: r_puntero64)
  0x48 0x21 0xF8 -> AND RAX, RDI (origen: r_destino64)
  0x48 0x21 0xE0 -> AND RAX, RSP (origen: r_pila64)

Destino: r_base64 (RBX)
  0x48 0x21 0xC3 -> AND RBX, RAX (origen: r_acumulador64)
  0x48 0x21 0xDB -> AND RBX, RBX (origen: r_base64)
  0x48 0x21 0xCB -> AND RBX, RCX (origen: r_contador64)
  0x48 0x21 0xD3 -> AND RBX, RDX (origen: r_datos64)
  0x48 0x21 0xF3 -> AND RBX, RSI (origen: r_puntero64)
  0x48 0x21 0xFB -> AND RBX, RDI (origen: r_destino64)
  0x48 0x21 0xE3 -> AND RBX, RSP (origen: r_pila64)

Destino: r_contador64 (RCX)
  0x48 0x21 0xC1 -> AND RCX, RAX (origen: r_acumulador64)
  0x48 0x21 0xD9 -> AND RCX, RBX (origen: r_base64)
  0x48 0x21 0xC9 -> AND RCX, RCX (origen: r_contador64)
  0x48 0x21 0xD1 -> AND RCX, RDX (origen: r_datos64)
  0x48 0x21 0xF1 -> AND RCX, RSI (origen: r_puntero64)
  0x48 0x21 0xF9 -> AND RCX, RDI (origen: r_destino64)
  0x48 0x21 0xE1 -> AND RCX, RSP (origen: r_pila64)

Destino: r_datos64 (RDX)
  0x48 0x21 0xC2 -> AND RDX, RAX (origen: r_acumulador64)
  0x48 0x21 0xDA -> AND RDX, RBX (origen: r_base64)
  0x48 0x21 0xCA -> AND RDX, RCX (origen: r_contador64)
  0x48 0x21 0xD2 -> AND RDX, RDX (origen: r_datos64)
  0x48 0x21 0xF2 -> AND RDX, RSI (origen: r_puntero64)
  0x48 0x21 0xFA -> AND RDX, RDI (origen: r_destino64)
  0x48 0x21 0xE2 -> AND RDX, RSP (origen: r_pila64)

Destino: r_puntero64 (RSI)
  0x48 0x21 0xC6 -> AND RSI, RAX (origen: r_acumulador64)
  0x48 0x21 0xDE -> AND RSI, RBX (origen: r_base64)
  0x48 0x21 0xCE -> AND RSI, RCX (origen: r_contador64)
  0x48 0x21 0xD6 -> AND RSI, RDX (origen: r_datos64)
  0x48 0x21 0xF6 -> AND RSI, RSI (origen: r_puntero64)
  0x48 0x21 0xFE -> AND RSI, RDI (origen: r_destino64)
  0x48 0x21 0xE6 -> AND RSI, RSP (origen: r_pila64)

Destino: r_destino64 (RDI)
  0x48 0x21 0xC7 -> AND RDI, RAX (origen: r_acumulador64)
  0x48 0x21 0xDF -> AND RDI, RBX (origen: r_base64)
  0x48 0x21 0xCF -> AND RDI, RCX (origen: r_contador64)
  0x48 0x21 0xD7 -> AND RDI, RDX (origen: r_datos64)
  0x48 0x21 0xF7 -> AND RDI, RSI (origen: r_puntero64)
  0x48 0x21 0xFF -> AND RDI, RDI (origen: r_destino64)
  0x48 0x21 0xE7 -> AND RDI, RSP (origen: r_pila64)

Destino: r_pila64 (RSP)
  0x48 0x21 0xC4 -> AND RSP, RAX (origen: r_acumulador64)
  0x48 0x21 0xDC -> AND RSP, RBX (origen: r_base64)
  0x48 0x21 0xCC -> AND RSP, RCX (origen: r_contador64)
  0x48 0x21 0xD4 -> AND RSP, RDX (origen: r_datos64)
  0x48 0x21 0xF4 -> AND RSP, RSI (origen: r_puntero64)
  0x48 0x21 0xFC -> AND RSP, RDI (origen: r_destino64)
  0x48 0x21 0xE4 -> AND RSP, RSP (origen: r_pila64)


31. o (OR - O Lógico)

Operandos: 
- Inmediato a Registro: o <numero>, <registro_destino> (ej. o 128, r_acumulador64)
- Registro a Registro: o <registro_origen>, <registro_destino> (ej. o r_base64, r_acumulador64)

Descripción: Realiza la operación lógica OR (O) bit a bit entre el origen y el destino, guardando el resultado en el destino. Se usa comúnmente para encender bits específicos (ponerlos a 1).

--- MODO INMEDIATO (Inmediato a Registro) ---
Realiza la operación OR con un valor numérico (32 bits) directamente al registro.

Registros de 64 bits:
  0x48 0x0D + imm32 -> OR RAX, imm32 (Destino: r_acumulador64)
  0x48 0x81 0xCB + imm32 -> OR RBX, imm32 (Destino: r_base64)
  0x48 0x81 0xC9 + imm32 -> OR RCX, imm32 (Destino: r_contador64)
  0x48 0x81 0xCA + imm32 -> OR RDX, imm32 (Destino: r_datos64)
  0x48 0x81 0xCE + imm32 -> OR RSI, imm32 (Destino: r_puntero64)
  0x48 0x81 0xCF + imm32 -> OR RDI, imm32 (Destino: r_destino64)
  0x48 0x81 0xCC + imm32 -> OR RSP, imm32 (Destino: r_pila64)

--- MODO REGISTRO A REGISTRO (64 bits) ---
Opcode base 0x48 0x09.

Destino: r_acumulador64 (RAX)
  0x48 0x09 0xD8 -> OR RAX, RBX (origen: r_base64)
  0x48 0x09 0xC0 -> OR RAX, RAX (origen: r_acumulador64)
  0x48 0x09 0xC8 -> OR RAX, RCX (origen: r_contador64)
  0x48 0x09 0xD0 -> OR RAX, RDX (origen: r_datos64)
  0x48 0x09 0xF0 -> OR RAX, RSI (origen: r_puntero64)
  0x48 0x09 0xF8 -> OR RAX, RDI (origen: r_destino64)
  0x48 0x09 0xE0 -> OR RAX, RSP (origen: r_pila64)

Destino: r_base64 (RBX)
  0x48 0x09 0xC3 -> OR RBX, RAX (origen: r_acumulador64)
  0x48 0x09 0xDB -> OR RBX, RBX (origen: r_base64)
  0x48 0x09 0xCB -> OR RBX, RCX (origen: r_contador64)
  0x48 0x09 0xD3 -> OR RBX, RDX (origen: r_datos64)
  0x48 0x09 0xF3 -> OR RBX, RSI (origen: r_puntero64)
  0x48 0x09 0xFB -> OR RBX, RDI (origen: r_destino64)
  0x48 0x09 0xE3 -> OR RBX, RSP (origen: r_pila64)

Destino: r_contador64 (RCX)
  0x48 0x09 0xC1 -> OR RCX, RAX (origen: r_acumulador64)
  0x48 0x09 0xD9 -> OR RCX, RBX (origen: r_base64)
  0x48 0x09 0xC9 -> OR RCX, RCX (origen: r_contador64)
  0x48 0x09 0xD1 -> OR RCX, RDX (origen: r_datos64)
  0x48 0x09 0xF1 -> OR RCX, RSI (origen: r_puntero64)
  0x48 0x09 0xF9 -> OR RCX, RDI (origen: r_destino64)
  0x48 0x09 0xE1 -> OR RCX, RSP (origen: r_pila64)

Destino: r_datos64 (RDX)
  0x48 0x09 0xC2 -> OR RDX, RAX (origen: r_acumulador64)
  0x48 0x09 0xDA -> OR RDX, RBX (origen: r_base64)
  0x48 0x09 0xCA -> OR RDX, RCX (origen: r_contador64)
  0x48 0x09 0xD2 -> OR RDX, RDX (origen: r_datos64)
  0x48 0x09 0xF2 -> OR RDX, RSI (origen: r_puntero64)
  0x48 0x09 0xFA -> OR RDX, RDI (origen: r_destino64)
  0x48 0x09 0xE2 -> OR RDX, RSP (origen: r_pila64)

Destino: r_puntero64 (RSI)
  0x48 0x09 0xC6 -> OR RSI, RAX (origen: r_acumulador64)
  0x48 0x09 0xDE -> OR RSI, RBX (origen: r_base64)
  0x48 0x09 0xCE -> OR RSI, RCX (origen: r_contador64)
  0x48 0x09 0xD6 -> OR RSI, RDX (origen: r_datos64)
  0x48 0x09 0xF6 -> OR RSI, RSI (origen: r_puntero64)
  0x48 0x09 0xFE -> OR RSI, RDI (origen: r_destino64)
  0x48 0x09 0xE6 -> OR RSI, RSP (origen: r_pila64)

Destino: r_destino64 (RDI)
  0x48 0x09 0xC7 -> OR RDI, RAX (origen: r_acumulador64)
  0x48 0x09 0xDF -> OR RDI, RBX (origen: r_base64)
  0x48 0x09 0xCF -> OR RDI, RCX (origen: r_contador64)
  0x48 0x09 0xD7 -> OR RDI, RDX (origen: r_datos64)
  0x48 0x09 0xF7 -> OR RDI, RSI (origen: r_puntero64)
  0x48 0x09 0xFF -> OR RDI, RDI (origen: r_destino64)
  0x48 0x09 0xE7 -> OR RDI, RSP (origen: r_pila64)

Destino: r_pila64 (RSP)
  0x48 0x09 0xC4 -> OR RSP, RAX (origen: r_acumulador64)
  0x48 0x09 0xDC -> OR RSP, RBX (origen: r_base64)
  0x48 0x09 0xCC -> OR RSP, RCX (origen: r_contador64)
  0x48 0x09 0xD4 -> OR RSP, RDX (origen: r_datos64)
  0x48 0x09 0xF4 -> OR RSP, RSI (origen: r_puntero64)
  0x48 0x09 0xFC -> OR RSP, RDI (origen: r_destino64)
  0x48 0x09 0xE4 -> OR RSP, RSP (origen: r_pila64)

32. cargar (MOV reg, [reg + offset] / MOV reg, [etiqueta])

Operandos: 
- Etiqueta a Registro: cargar variable, r_acumulador64
- Memoria Indirecta a Registro: cargar [r_base64+8], r_acumulador64

Descripción: Carga un valor de 64 bits desde la memoria al registro destino. Si se le pasa una etiqueta, usa direccionamiento relativo a la instrucción (RIP-relative, opcode 0x8B 0x05/0x1D). Si se le pasa un registro entre corchetes, usa direccionamiento indirecto con soporte para offsets (sumando 0x80 al ModR/M).

Opcode base 0x48 0x8B (MOV reg, r/m64):

[MODO ETIQUETA - RIP Relative]
0x48 0x8B 0x05 + disp32 -> MOV RAX, [RIP + disp32]
0x48 0x8B 0x1D + disp32 -> MOV RBX, [RIP + disp32]

[MODO INDIRECTO - Con Offset]
Ejemplo: cargar [r_base64+8], r_acumulador64
0x48 0x8B 0x83 0x08 0x00 0x00 0x00 -> MOV RAX, [RBX + 8]

33. guardar_doble_puntero (MOV DWORD PTR [reg + offset], reg32)

Operandos: Registro_origen_32, [Registro_direccion + offset] (ej. guardar_doble_puntero r_acumulador32, [r_base64+4]).

Descripción: Guarda 32 bits (4 bytes) del registro de origen en la dirección de memoria apuntada por el registro base, con soporte para desplazamientos (offsets). Es fundamental para manejar arrays de enteros (int) o estructuras de datos.
A diferencia de la versión de 64 bits, NO usa el prefijo REX (0x48).

Opcode base 0x89 (MOV r/m32, r32):
Ejemplo sin offset: guardar_doble_puntero r_acumulador32, [r_base64] 
  -> 0x89 0x03
Ejemplo con offset: guardar_doble_puntero r_acumulador32, [r_base64+4]
  -> 0x89 0x83 0x04 0x00 0x00 0x00

[Nota Técnica sobre Offsets]: 
Para activar el direccionamiento con desplazamiento, el ensamblador coge el byte ModR/M base (ej. 0x03) y le aplica un OR a nivel de bits con 0x80 (0x03 | 0x80 = 0x83). Eso le indica a la CPU que los siguientes 4 bytes son el offset (disp32).

34. cargar_doble_puntero (MOV reg32, DWORD PTR [reg + offset])

Operandos: [Registro_direccion + offset], Registro_destino_32 (ej. cargar_doble_puntero [r_base64+4], r_acumulador32).

Descripción: Carga 32 bits (4 bytes) desde la dirección de memoria apuntada por el registro base (con soporte para offsets) hacia el registro destino. 
Ideal para leer enteros de memoria.
A diferencia de la versión de 64 bits, NO usa el prefijo REX (0x48).

Opcode base 0x8B (MOV r32, r/m32):
Ejemplo sin offset: cargar_doble_puntero [r_base64], r_acumulador32
  -> 0x8B 0x03
Ejemplo con offset: cargar_doble_puntero [r_base64+4], r_acumulador32
  -> 0x8B 0x83 0x04 0x00 0x00 0x00