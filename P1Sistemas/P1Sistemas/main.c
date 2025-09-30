#define F_CPU 16000000UL  // Debe estar definido antes de incluir delay.h
#include <avr/io.h>
#include <util/delay.h>

// Definición de pines para ADC0831 (corregidos según el esquemático)
#define CS_PIN  PD2    // Chip Select - PIN 1 del ADC0831
#define CLK_PIN PD4    // Clock - PIN 7 del ADC0831
#define DO_PIN  PD3    // Data Out - PIN 6 del ADC0831

// Función para leer un valor del ADC0831 (8 bits) - protocolo revisado
uint8_t readADC0831(void) {
    uint8_t value = 0;
    
    // Comenzar la conversión (CS a bajo)
    PORTD &= ~(1 << CS_PIN);
    _delay_us(10);  // Mayor delay para estabilidad
    
    // Primer pulso de reloj para iniciar conversión
    PORTD |= (1 << CLK_PIN);
    _delay_us(10);
    PORTD &= ~(1 << CLK_PIN);
    _delay_us(10);
    
    // El primer bit es nulo en ADC0831, lo ignoramos
    PORTD |= (1 << CLK_PIN);
    _delay_us(10);
    PORTD &= ~(1 << CLK_PIN);
    _delay_us(10);
    
    // Leer los 8 bits, empezando por el MSB
    for (int i = 7; i >= 0; i--) {
        // Generar pulso de reloj
        PORTD |= (1 << CLK_PIN);
        _delay_us(10);
        
        // Leer el bit de datos
        if (PIND & (1 << DO_PIN))
            value |= (1 << i);
            
        PORTD &= ~(1 << CLK_PIN);
        _delay_us(10);
    }
    
    // Finalizar la conversión (CS a alto)
    PORTD |= (1 << CS_PIN);
    
    return value;
}

// Función para actualizar el LED-BARGRAPH
void updateLEDs(uint8_t value) {
    // Mapear el valor del ADC (0-255) a los LEDs
    // Si el valor es 0, todos apagados
    // Si el valor es 255, todos encendidos
    
    // Establece directamente los bits para los LEDs
    PORTB = value;
}

int main(void) {
    // Configurar los pines del ADC como salidas/entradas
    DDRD |= (1 << CS_PIN) | (1 << CLK_PIN);  // CS y CLK como salidas
    DDRD &= ~(1 << DO_PIN);                 // DO como entrada
    PORTD |= (1 << DO_PIN);                 // Activar pull-up en DO
    
    // Establecer estados iniciales
    PORTD |= (1 << CS_PIN);   // CS inicialmente en alto
    PORTD &= ~(1 << CLK_PIN); // CLK inicialmente en bajo
    
    // Configurar todo el Puerto B como salida para los LEDs
    DDRB = 0xFF;  // 0xFF = 0b11111111 (todos los pines como salida)
    
    // Inicializar LEDs apagados
    PORTB = 0x00;
    
    // Esperar un poco al inicio para que todo se estabilice
    _delay_ms(100);
    
    // Bucle principal
    while (1) {
        // Leer valor del ADC (0-255)
        uint8_t adc_value = readADC0831();
        
        // Actualizar LEDs según el valor leído
        updateLEDs(adc_value);
        
        // Pequeño retardo
        _delay_ms(50);
    }
    
    return 0;
}