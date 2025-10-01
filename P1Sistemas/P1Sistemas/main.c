// Usar 1MHz porque CLKDIV8 está habilitado (8MHz/8)
#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

// Recalcular para asegurar la precisión del baud rate
#define BAUD 2400
#define MYUBRR (F_CPU/16/BAUD-1)

// Función para inicializar USART
void USART_Init(unsigned int ubrr) {
    // Configurar baud rate
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;

    // Habilitar transmisor
    UCSR0B = (1<<TXEN0);

    // Configurar formato: 8 bits de datos, 1 bit de stop, sin paridad
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

// Función para transmitir un carácter
void USART_Transmit(unsigned char data) {
    // Esperar a que el buffer de transmisión esté vacío
    while (!(UCSR0A & (1<<UDRE0)));

    // Poner datos en el buffer de transmisión
    UDR0 = data;
}

// Función para enviar una cadena de caracteres
void USART_TransmitString(char* str) {
    while (*str) {
        USART_Transmit(*str++);
    }
}

int main(void) {
    // Configurar PORTB como salida para LEDs
    DDRB = 0xFF;
    PORTB = 0x55; // Patrón alternado para verificar actividad
    
    // Inicializar USART con valor calculado correctamente
    USART_Init(MYUBRR);
    
    // Retardo inicial para estabilización
    _delay_ms(1000);
    
    while (1) {
        // Enviar mensaje "Hola mundo"
        USART_TransmitString("Hola mundo\r\n");
        
        // Alternar LEDs para indicar actividad
        PORTB = ~PORTB;
        
        // Esperar un segundo antes de enviar de nuevo
        _delay_ms(1000);
    }
    
    return 0;
}