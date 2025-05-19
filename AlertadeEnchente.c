#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "hardware/pwm.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Matriz de LEDs
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

#define buzzer 10    // Pino do buzzer A
#define led_RED 13   // Red=13, Blue=12, Green=11
#define led_GREEN 11 // Red=13, Blue=12, Green=11
#define botao_pinA 5 // Botão A = 5, Botão B = 6 , BotãoJoy = 22

// Variaveis globais
bool Noturno = false;
const char* cores[] = {
    "Verde",
    "Amare",
    "Verme",
    "Notur"
};
uint8_t estado = 0;
bool led_buffer[4][NUM_PIXELS] = {
    {
        0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 0, 0, 0  
    },
    {
        0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0   
    },
    {
        0, 0, 0, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0  
    },
    {
        0, 0, 0, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 0, 0, 0  
    }
};

// Funcoes para serem chamadas
// Funções para matriz LEDS
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_one_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[estado /*variavel do arrey do buffer*/][i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}
// Funcoes de programa do FreeRTOS
//funcao para acender semaforo
void vSemaforoTask()
{
    // configuracao do PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    //iniciacao pinos
    gpio_init(led_RED);
    gpio_set_dir(led_RED, GPIO_OUT);
    gpio_init(led_GREEN);
    gpio_set_dir(led_GREEN, GPIO_OUT);
    gpio_put(led_RED, false);
    gpio_put(led_GREEN, false);
    //iniciacao buzzer
    gpio_set_function(buzzer, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(buzzer);
    pwm_set_wrap(slice_num, 4096);
    // Define o clock divider como 440 (nota lá para o buzzer)
    pwm_set_clkdiv(slice_num, 440.0f);
    pwm_set_enabled(slice_num, true);

    while (true)
    {
        //sinal verde
        if (!Noturno)
        {
            estado = 0;
            set_one_led(0, 5, 0);
            gpio_put(led_GREEN, true);
            gpio_put(led_RED, false);
            pwm_set_gpio_level(buzzer, 2048);
            vTaskDelay(pdMS_TO_TICKS(150));
            pwm_set_gpio_level(buzzer, 0);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
        //sinal amarelo
        if (!Noturno)
        {
            estado = 1;
            set_one_led(5, 5, 0);
            gpio_put(led_RED, true);
            gpio_put(led_GREEN, true);
            for (int i = 0; i < 4; i++)
            {
                pwm_set_gpio_level(buzzer, 2048);
                vTaskDelay(pdMS_TO_TICKS(200));
                pwm_set_gpio_level(buzzer, 0);
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            vTaskDelay(pdMS_TO_TICKS(1250));
        }
        //sinal vermelho
        if (!Noturno)
        {
            estado = 2;
            set_one_led(5, 0, 0);
            gpio_put(led_GREEN, false);
            gpio_put(led_RED, true);
            pwm_set_gpio_level(buzzer, 2048);
            vTaskDelay(pdMS_TO_TICKS(500));
            pwm_set_gpio_level(buzzer, 0);
            vTaskDelay(pdMS_TO_TICKS(1500));
            gpio_put(led_RED, false);
        }
        //modo noturno
        if (Noturno)
        {
            estado = 3;
            set_one_led(5, 5, 0);
            pwm_set_gpio_level(buzzer, 2048);
            gpio_put(led_RED, true);
            gpio_put(led_GREEN, true);
            vTaskDelay(pdMS_TO_TICKS(500));
            set_one_led(0, 0, 0);
            pwm_set_gpio_level(buzzer, 0);
            gpio_put(led_RED, false);
            gpio_put(led_GREEN, false);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}
//funcao para verificar botao A
void vModoTask()
{
    gpio_init(botao_pinA);
    gpio_set_dir(botao_pinA, GPIO_IN);
    gpio_pull_up(botao_pinA);
    while (true)
    {
        if (gpio_get(botao_pinA))
        {
            Noturno = false;
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        else
        {
            Noturno = true;
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}
//funcao para modificar o display
void vDisplay3Task()
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    bool cor = true;
    while (true)
    {
        ssd1306_fill(&ssd, !cor);                            // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);        // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);             // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);             // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);   // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);    // Desenha uma string
        ssd1306_draw_string(&ssd, "FreeRTOS - Wil", 10, 28);     // Desenha uma string
        ssd1306_draw_string(&ssd, "   SEMAFORO", 10, 41); // Desenha uma string
        ssd1306_draw_string(&ssd, cores[estado], 44, 52);            // Desenha uma string
        ssd1306_send_data(&ssd);                             // Atualiza o display
        sleep_ms(735);
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();
    //chamadas de funcoes
    xTaskCreate(vSemaforoTask, "Semaforo Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplay3Task, "Cont Task Disp3", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vModoTask, "Modo norturno Task", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler(); // comeca as chamadas das funcoes
    panic_unsupported(); // caso aconteca algum desastre com o programa
}
