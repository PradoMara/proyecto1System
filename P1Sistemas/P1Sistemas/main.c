// Usar 1MHz porque CLKDIV8 está habilitado (8MHz/8)
#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

// Recalcular para asegurar la precisión del baud rate
#define BAUD 2400
#define MYUBRR (F_CPU/16/BAUD-1)

// Definición de pines para ADC0831
#define CS_PIN  PD2    // Chip Select
#define CLK_PIN PD4    // Clock
#define DO_PIN  PD3    // Data Out

// Función para inicializar USART
void USART_Init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1<<TXEN0);
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

// Función para transmitir un carácter
void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

// Función para enviar una cadena de caracteres
void USART_TransmitString(char* str) {
    while (*str) {
        USART_Transmit(*str++);
    }
}

// Función para convertir entero a cadena
void uint8_to_string(uint8_t value, char* str) {
    uint8_t i = 0, temp;
    
    // Caso especial para cero
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    // Convertir dígitos
    while (value > 0) {
        temp = value % 10;
        str[i++] = temp + '0';
        value /= 10;
    }
    
    // Terminar cadena
    str[i] = '\0';
    
    // Invertir la cadena
    for (uint8_t j = 0; j < i/2; j++) {
        temp = str[j];
        str[j] = str[i-j-1];
        str[i-j-1] = temp;
    }
}

// Función para leer un valor del ADC0831 (8 bits)
uint8_t readADC0831(void) {
    uint8_t value = 0;
    
    // Comenzar la conversión (CS a bajo)
    PORTD &= ~(1 << CS_PIN);
    _delay_us(10);
    
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

int main(void) {
    char buffer[10]; // Buffer para formatear valores numéricos
    
    // Configurar los pines del ADC
    DDRD |= (1 << CS_PIN) | (1 << CLK_PIN);  // CS y CLK como salidas
    DDRD &= ~(1 << DO_PIN);                  // DO como entrada
    PORTD |= (1 << DO_PIN);                  // Activar pull-up en DO
    
    // Establecer estados iniciales
    PORTD |= (1 << CS_PIN);    // CS inicialmente en alto
    PORTD &= ~(1 << CLK_PIN);  // CLK inicialmente en bajo
    
    // Configurar PORTB como salida para LEDs
    DDRB = 0xFF;
    PORTB = 0x00;  // Inicializar apagados
    
    // Inicializar USART
    USART_Init(MYUBRR);
    
    // Retardo inicial para estabilización
    _delay_ms(100);
    
    // Mensaje de inicio
    USART_TransmitString("Iniciando lectura del ADC\r\n");
    
    while (1) {
        // Leer valor del ADC
        uint8_t adc_value = readADC0831();
        
        // Actualizar LEDs según valor leído
        PORTB = adc_value;
        
        // Enviar valor numérico por UART
        USART_TransmitString("Valor: ");
        uint8_to_string(adc_value, buffer);
        USART_TransmitString(buffer);
        
        // Mostrar representación binaria de LEDs
        USART_TransmitString(" | LEDs: ");
        for (int i = 7; i >= 0; i--) {
            if (adc_value & (1 << i)) {
                USART_TransmitString("1");
            } else {
                USART_TransmitString("0");
            }
        }
        USART_TransmitString("\r\n");
        
        // Esperar antes de la siguiente lectura
        _delay_ms(500);
    }
    
    return 0;
}