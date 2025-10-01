import serial

# Ajusta el puerto COM según corresponda
ser = serial.Serial('COM2', 2400, timeout=1)

while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').rstrip()
        print(line)