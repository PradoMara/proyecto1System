/*
 * P1Sistemas.c
 * Sistema de Adquisición de Datos con ATmega328P
 * Created: 27-09-2025 13:53:21
 * Author : dp877
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define F_CPU 16000000UL        // Frecuencia del cristal 16MHz

// Definición de pines para ADC0831 en ATmega328P
#define ADC_CS_PORT   PORTC    // Puerto C
#define ADC_CS_DDR    DDRC
#define ADC_CS_PIN    PC0      // Pin PC0 - Chip Select

#define ADC_CLK_PORT  PORTC    // Puerto C
#define ADC_CLK_DDR   DDRC
#define ADC_CLK_PIN   PC1      // Pin PC1 - Clock

#define ADC_DO_PORT   PORTC    // Puerto C
#define ADC_DO_DDR    DDRC
#define ADC_DO_PIN    PINC     // Pin PC2 - Data Out (lectura)
#define ADC_DO_BIT    PC2

// Definición de pines para LEDs bargraph (Puerto D)
#define LED_PORT      PORTD
#define LED_DDR       DDRD

// Definición para UART (ya definido en ATmega328P)
#define UART_BAUD 2400
#define UBRR_VALUE ((F_CPU/(16UL*UART_BAUD))-1)

// Variables globales
unsigned char adc_value = 0;
unsigned char led_pattern = 0;
unsigned char system_mode = 3;  // Modo 3 como se especifica en el diagrama

// Prototipos de funciones
void init_system(void);
void init_uart(void);
unsigned char read_adc0831(void);
void display_led_bargraph(unsigned char value);
void send_uart_data(unsigned char data);
unsigned char receive_uart_data(void);
void delay_ms(unsigned int ms);
unsigned char abs_diff(unsigned char a, unsigned char b);

// Función de inicialización del sistema
void init_system(void) {
    // Configurar Puerto C (ADC0831)
    ADC_CS_DDR |= (1 << ADC_CS_PIN);    // CS como salida
    ADC_CLK_DDR |= (1 << ADC_CLK_PIN);  // CLK como salida
    ADC_DO_DDR &= ~(1 << ADC_DO_BIT);   // DO como entrada
    
    ADC_CS_PORT |= (1 << ADC_CS_PIN);   // CS inicialmente alto
    ADC_CLK_PORT &= ~(1 << ADC_CLK_PIN); // CLK inicialmente bajo
    
    // Configurar Puerto D (LEDs bargraph)
    LED_DDR = 0xFF;      // Todo el puerto D como salida para LEDs
    LED_PORT = 0x00;     // Apagar todos los LEDs inicialmente
}

// Función para leer ADC0831
unsigned char read_adc0831(void) {
    unsigned char i;
    unsigned char data = 0;
    
    // Iniciar conversión - CS bajo
    ADC_CS_PORT &= ~(1 << ADC_CS_PIN);
    _delay_us(1);
    
    // Generar 8 pulsos de reloj y leer datos
    for(i = 0; i < 8; i++) {
        // Clock alto
        ADC_CLK_PORT |= (1 << ADC_CLK_PIN);
        _delay_us(1);
        
        // Leer bit de datos
        data <<= 1;
        if(ADC_DO_PIN & (1 << ADC_DO_BIT)) {
            data |= 1;
        }
        
        // Clock bajo
        ADC_CLK_PORT &= ~(1 << ADC_CLK_PIN);
        _delay_us(1);
    }
    
    // CS alto para terminar conversión
    ADC_CS_PORT |= (1 << ADC_CS_PIN);
    
    return data;
}

// Función para mostrar valor en bargraph LED
void display_led_bargraph(unsigned char value) {
    unsigned char led_count;
    unsigned char led_output = 0;
    
    // Convertir valor ADC (0-255) a número de LEDs (0-8)
    led_count = (value * 8) / 255;
    
    // Crear patrón de LEDs encendidos
    for(unsigned char i = 0; i < led_count; i++) {
        led_output |= (1 << i);
    }
    
    // Enviar patrón al puerto D
    LED_PORT = led_output;
}

// Función de inicialización UART para 2400bps usando hardware USART
void init_uart(void) {
    // Configurar baudrate para 2400bps
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)UBRR_VALUE;
    
    // Habilitar transmisor y receptor
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    
    // Configurar formato: 8 bits de datos, 1 bit de parada
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Función para enviar un byte por UART hardware
void send_uart_data(unsigned char data) {
    // Esperar que el buffer de transmisión esté vacío
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Enviar datos
    UDR0 = data;
}

// Función para recibir un byte por UART (opcional)
unsigned char receive_uart_data(void) {
    // Esperar que lleguen datos
    while (!(UCSR0A & (1 << RXC0)));
    
    // Retornar datos recibidos
    return UDR0;
}

// Función de delay en milisegundos
void delay_ms(unsigned int ms) {
    for(unsigned int i = 0; i < ms; i++) {
        _delay_ms(1);
    }
}

// Función para calcular diferencia absoluta
unsigned char abs_diff(unsigned char a, unsigned char b) {
    if(a > b) {
        return a - b;
    } else {
        return b - a;
    }
}

int main(void)
{
    unsigned char previous_value = 0;
    unsigned char sample_counter = 0;
    
    // Inicializar sistema
    init_system();
    init_uart();
    
    // Mensaje de inicio por UART
    const char startup_msg[] = "Sistema ADC iniciado - Modo 3\r\n";
    for(unsigned char i = 0; startup_msg[i] != '\0'; i++) {
        send_uart_data(startup_msg[i]);
    }
    
    while (1) 
    {
        // Leer valor del ADC0831
        adc_value = read_adc0831();
        
        // Mostrar en bargraph LED
        display_led_bargraph(adc_value);
        
        // Enviar datos por UART solo si hay cambio significativo o cada 10 muestras
        if((abs_diff(adc_value, previous_value) > 2) || (sample_counter >= 10)) {
            // Enviar valor como texto hexadecimal
            send_uart_data('0');
            send_uart_data('x');
            
            // Convertir a hexadecimal y enviar
            unsigned char high_nibble = (adc_value >> 4) & 0x0F;
            unsigned char low_nibble = adc_value & 0x0F;
            
            // Enviar nibble alto
            if(high_nibble < 10) {
                send_uart_data('0' + high_nibble);
            } else {
                send_uart_data('A' + high_nibble - 10);
            }
            
            // Enviar nibble bajo
            if(low_nibble < 10) {
                send_uart_data('0' + low_nibble);
            } else {
                send_uart_data('A' + low_nibble - 10);
            }
            
            send_uart_data('\r');
            send_uart_data('\n');
            
            previous_value = adc_value;
            sample_counter = 0;
        } else {
            sample_counter++;
        }
        
        // Delay para estabilidad (100ms)
        _delay_ms(100);
    }
}

