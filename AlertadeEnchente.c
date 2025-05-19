#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "hardware/pwm.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_JOYSTICK_X 26
#define ADC_JOYSTICK_Y 27
#define LED_BLUE 12
#define LED_GREEN 11
#define tam_quad 10

typedef struct
{
    uint16_t x_pos;
    uint16_t y_pos;
} joystick_data_t;

QueueHandle_t xQueueJoystickData;

// Matriz de LEDs
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

#define buzzer 10    // Pino do buzzer A
#define led_RED 13   // Red=13, Blue=12, Green=11
#define led_GREEN 11 // Red=13, Blue=12, Green=11
#define botao_pinA 5 // Botão A = 5, Botão B = 6 , BotãoJoy = 22

// Variaveis globais
uint8_t estado = 0;
bool led_buffer[2][NUM_PIXELS] = {
    {0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0,
     0, 1, 0, 1, 0,
     1, 0, 1, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1}};

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
void vJoystickTask(void *params)
{
    adc_gpio_init(ADC_JOYSTICK_Y);
    adc_gpio_init(ADC_JOYSTICK_X);
    adc_init();

    joystick_data_t joydata;

    while (true)
    {
        adc_select_input(0); // GPIO 26 = ADC0
        joydata.y_pos = adc_read();

        adc_select_input(1); // GPIO 27 = ADC1
        joydata.x_pos = adc_read();

        xQueueSend(xQueueJoystickData, &joydata, 0); // Envia o valor do joystick para a fila
        vTaskDelay(pdMS_TO_TICKS(100));              // 10 Hz de leitura
    }
}

void vDisplayTask(void *params)
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    joystick_data_t joydata;
    bool cor = true;
    char str_x[5]; // Buffer para armazenar a string
    char str_y[5]; // Buffer para armazenar a string
    float porcent_x, porcent_y;
    while (true)
    {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE)
        {
            porcent_x = (int16_t)joydata.x_pos * 0.024;
            porcent_y = (int16_t)joydata.y_pos * 0.024;
            sprintf(str_x, "%1.0f", porcent_x);               // Converte o inteiro em string
            sprintf(str_y, "%1.0f", porcent_y);               // Converte o float em string
            ssd1306_fill(&ssd, !cor);                         // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);     // Desenha um retângulo
            ssd1306_line(&ssd, 3, 25, 123, 25, cor);          // Desenha uma linha
            ssd1306_line(&ssd, 3, 37, 123, 37, cor);          // Desenha uma linha
            ssd1306_draw_string(&ssd, "Monitoramento", 8, 6); // Desenha uma string
            ssd1306_draw_string(&ssd, "Chuva:", 10, 16);      // Desenha uma string
            ssd1306_draw_string(&ssd, str_x, 60, 16);         // Desenha uma string
            ssd1306_draw_string(&ssd, "Agua:", 10, 28);       // Desenha uma string
            ssd1306_draw_string(&ssd, str_y, 60, 28);         // Desenha uma string
            if (porcent_x > 80 || porcent_y > 70){
                ssd1306_draw_string(&ssd, "VALORES   !!!", 10, 40);       // Desenha uma string
                ssd1306_draw_string(&ssd, "ALARMANTES!!!", 10, 50);       // Desenha uma string
            }
            ssd1306_send_data(&ssd);                          // Atualiza o display
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Atualiza a cada 50ms
    }
}

void vAlertas(void *params)
{
    // configuracao do PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    // iniciacao pinos
    gpio_init(led_RED);
    gpio_set_dir(led_RED, GPIO_OUT);
    gpio_init(led_GREEN);
    gpio_set_dir(led_GREEN, GPIO_OUT);
    gpio_put(led_RED, false);
    gpio_put(led_GREEN, false);
    // iniciacao buzzer
    gpio_set_function(buzzer, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(buzzer);
    pwm_set_wrap(slice_num, 4096);
    // Define o clock divider como 440 (nota lá para o buzzer)
    pwm_set_clkdiv(slice_num, 440.0f);
    pwm_set_enabled(slice_num, true);
    float porcent_x, porcent_y;
    joystick_data_t joydata;
    while (true)
    {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE)
        {
            porcent_x = (int16_t)joydata.x_pos * 0.024;
            porcent_y = (int16_t)joydata.y_pos * 0.024;
            if (porcent_x < 80 && porcent_y < 70)
            {
                estado = 0;
                set_one_led(0, 5, 0);
                gpio_put(led_GREEN, true);
                gpio_put(led_RED, false);
                pwm_set_gpio_level(buzzer, 0);
            }
            else
            {
                estado = 1;
                set_one_led(5, 0, 0);
                gpio_put(led_GREEN, false);
                gpio_put(led_RED, true);
                pwm_set_gpio_level(buzzer, 2048);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Atualiza a cada 50ms
    }
}

// Modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Ativa BOOTSEL via botão
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    stdio_init_all();

    // Cria a fila para compartilhamento de valor do joystick
    xQueueJoystickData = xQueueCreate(5, sizeof(joystick_data_t));

    // Criação das tasks
    xTaskCreate(vJoystickTask, "Joystick Task", 256, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "Display Task", 512, NULL, 1, NULL);
    xTaskCreate(vAlertas, "Alertas", 256, NULL, 1, NULL);
    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}