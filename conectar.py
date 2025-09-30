import serial
import time

def main():
    # Configuración para VSPE COM2
    puerto = 'COM'  # Puerto virtual de VSPE
    baudrate = 2400
    
    try:
        # Conectar al puerto COM2
        ser = serial.Serial(puerto, baudrate, timeout=1)
        print(f"Conectado a {puerto} a {baudrate} baudios (VSPE)")
        print("Leyendo datos... (Ctrl+C para salir)")
        print("-" * 40)
        
        # Leer datos continuamente
        while True:
            if ser.in_waiting > 0:
                data = ser.readline().decode('utf-8').strip()
                if data:
                    print(f"Recibido: {data}")
            time.sleep(0.1)
            
    except serial.SerialException as e:
        print(f"Error de conexión: {e}")
        print("Verifica que VSPE esté ejecutándose y COM2 esté disponible")
    except KeyboardInterrupt:
        print("\nDesconectando...")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Conexión cerrada")

if __name__ == "__main__":
    main()  