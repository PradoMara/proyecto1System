import serial
import re
import time

try:
    # Ajusta el puerto COM según corresponda
    ser = serial.Serial('COM2', 2400, timeout=1)
    print("Conectado al puerto serial. Esperando datos...")
    print("Presiona Ctrl+C para terminar la conexión.")

    # Patrón para extraer el voltaje
    voltaje_pattern = re.compile(r'Voltaje: (\d+\.\d+) V')

    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            print(line)
            
            # Analizar voltaje si está presente (solo para futura referencia)
            match = voltaje_pattern.search(line)
            if match:
                voltaje = float(match.group(1))
        
        # Pequeña pausa para reducir uso de CPU
        time.sleep(0.01)

except KeyboardInterrupt:
    print("\nInterrupción detectada. Cerrando conexión serial...")
    if 'ser' in locals() and ser.is_open:
        ser.close()
    print("Conexión terminada correctamente.")

except serial.SerialException as e:
    print(f"\nError de conexión serial: {e}")
    if 'ser' in locals() and ser.is_open:
        ser.close()

finally:
    # Asegurar que el puerto se cierre incluso si hay otros errores
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Conexión serial cerrada.")