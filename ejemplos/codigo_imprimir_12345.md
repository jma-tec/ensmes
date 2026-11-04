.codigo
inicio:
# Número a imprimir en RAX
copiar 12345, r_acumulador64

# Cargar la dirección del buffer en RDI
cargar_direccion buffer, r_destino64

# Apuntar al final del buffer
sumar 19, r_destino64

# Bucle de conversión (guardando hacia atrás)
bucle:
comparar 0, r_acumulador64
saltar_si_igual preparar_impresion

o_exclusivo r_datos64, r_datos64
copiar 10, r_contador64
dividir r_contador64

sumar 48, r_datos64

# Guardamos SOLO 1 byte para no machacar el buffer
guardar_byte_puntero r_datos64, [r_destino64]

# ¡RETROCEDEMOS EL PUNTERO!
restar 1, r_destino64
saltar bucle

preparar_impresion:
# RDI apunta 1 byte ANTES del '1'.
# Sumamos 1 para que RDI apunte exactamente al '1'.
sumar 1, r_destino64

# Pasamos la dirección del '1' a RSI
copiar r_destino64, r_puntero64

# Imprimimos los 5 dígitos
copiar 5, r_datos64
copiar 1, r_acumulador64
copiar 1, r_destino64
interrumpir

# Ahora el salto de línea.
# RSI apunta al '1'. Si le sumamos 5, apunta justo al byte después del '5'.
sumar 5, r_puntero64
copiar 10, r_datos64
guardar_byte_puntero r_datos64, [r_puntero64]

# Imprimimos el salto de línea
copiar 1, r_datos64
copiar 1, r_acumulador64
copiar 1, r_destino64
interrumpir

terminar

.datos
buffer:
reservar_byte 20