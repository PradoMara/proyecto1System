import serial
import re

# Ajusta el puerto COM según corresponda
ser = serial.Serial('COM2', 2400, timeout=1)
print("Conectado al puerto serial. Esperando datos...")

# Patrón para extraer el voltaje
voltaje_pattern = re.compile(r'Voltaje: (\d+\.\d+) V')

while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').rstrip()
        print(line)
        
        # Analizar voltaje si está presente
        match = voltaje_pattern.search(line)
        if match:
            voltaje = float(match.group(1))
            # Aquí puedes hacer algo específico con el valor de voltaje
            # Por ejemplo, activar una alarma si supera cierto umbral
            if voltaje > 4.0:
                print("¡ALERTA! Voltaje alto detectado: {} V".format(voltaje))