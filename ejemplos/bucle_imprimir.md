.codigo
inicio:
# 1. Inicializamos el contador en r_base64 (RBX)
copiar 5, r_base64

bucle:
# 2. Comprobamos si el contador ha llegado a 0
comparar 0, r_base64
saltar_si_igual fin

# 3. --- AQUÍ VA EL CÓDIGO PARA IMPRIMIR LA 'A' ---
cargar_direccion buffer, r_puntero64
copiar 2, r_datos64
copiar 1, r_acumulador64
copiar 1, r_destino64
interrumpir


# 4. Restamos 1 al contador y repetimos
restar 1, r_base64
saltar bucle

fin:
terminar

.datos
buffer:
definir_byte 65 # 'A'
definir_byte 10 # '\n' (opcional, para que no se peguen las letras)