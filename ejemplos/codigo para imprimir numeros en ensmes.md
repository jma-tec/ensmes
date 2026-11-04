.codigo
inicio:
# Número a imprimir en RAX
copiar 12345, r_acumulador64

# Cargar la dirección del buffer en RDI (el INICIO del buffer)
cargar_direccion buffer, r_destino64

# Bucle de conversión
bucle:
comparar 0, r_acumulador64
saltar_si_igual preparar_impresion

# Limpiamos RDX y dividimos
o_exclusivo r_datos64, r_datos64
copiar 10, r_contador64
dividir r_contador64

# Convertir el resto a ASCII
sumar 48, r_datos64

# Guardamos el dígito en el buffer (usando tu instrucción de 64 bits)
# OJO: Escribe 8 bytes, pero el siguiente carácter lo sobreescribirá,
# dejando solo el último byte de cada dígito al final.
guardar_puntero r_datos64, [r_destino64]

# Avanzamos el puntero (en lugar de retroceder)
sumar 1, r_destino64
saltar bucle

preparar_impresion:
# RDI apunta al final del buffer (después del último dígito).
# Sabemos que hemos guardado 5 dígitos (12345).
# Vamos a imprimir 5 bytes desde el INICIO del buffer.
cargar_direccion buffer, r_puntero64

# Imprimimos 5 bytes (los 5 dígitos)
copiar 5, r_datos64
copiar 1, r_acumulador64
copiar 1, r_destino64
interrumpir

# Imprimimos el salto de línea
# Movemos RSI 5 posiciones desde el inicio
sumar 5, r_puntero64
copiar 10, r_datos64
guardar_puntero r_datos64, [r_puntero64] # Escribimos el \n

copiar 1, r_datos64
copiar 1, r_acumulador64
copiar 1, r_destino64
interrumpir

terminar

.datos
buffer:
reservar_byte 20