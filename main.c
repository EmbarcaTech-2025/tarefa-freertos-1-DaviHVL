// Bibliotecas necessárias
#include "FreeRTOS.h" // Biblioteca principal do FreeRTOS
#include "task.h" // Biblioteca de tarefas do FreeRTOS
#include <stdio.h> // Biblioteca padrão 
#include "pico/stdlib.h" // Biblioteca padrão pico
#include "hardware/pwm.h" // Biblioteca de PWM 
#include "hardware/gpio.h" // Biblioteca de GPIO 
// ----------------------------------------------------------------------------------------------------
// Definições de constantes

#define BUTTON_A 5 // Pino do Botão A
#define BUTTON_B 6 // Pino do Botão B

#define LED_PIN_GREEN 11 // Pino do LED Verde
#define LED_PIN_BLUE 12 // Pino do LED Azul
#define LED_PIN_RED 13  // Pino do LED Vermelho

#define BUZZER_PIN 21 // Pino do Buzzer
// ----------------------------------------------------------------------------------------------------
// Funções de configuração

// Função para configuração do LED RGB
void setup_leds(){
  gpio_init(LED_PIN_GREEN); // Inicializa o pino do LED Verde
  gpio_set_dir(LED_PIN_GREEN, GPIO_OUT); // Define o pino do LED Verde como saída

  gpio_init(LED_PIN_BLUE); // Inicializa o pino do LED Azul
  gpio_set_dir(LED_PIN_BLUE, GPIO_OUT); // Define o pino do LED Azul como saída

  gpio_init(LED_PIN_RED); // Inicializa o pino do LED Vermelho
  gpio_set_dir(LED_PIN_RED, GPIO_OUT); // Define o pino do LED Vermelho como saída
}

// Função para configuração do PWM do buzzer
void setup_buzzer(){
  // Identifica qual "slice" de PWM está conectado ao pino do buzzer
  uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);

  // Configura o pino do buzzer para a função de PWM
  gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM); 
  
  // Define o divisor de clock do PWM. Um valor baixo como 1.0f aumenta a frequência 
  // do PWM para fora da faixa audível, melhorando a qualidade do som.
  pwm_set_clkdiv(slice, 1.0f);  

  pwm_set_wrap(slice, 62500);

  // Define o nível inicial do canal (duty cycle) para 0.
  pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER_PIN), 0);

  // Ativa o PWM
  pwm_set_enabled(slice, true);
}

// Função para configuração dos botões
void setup_buttons() {
  gpio_init(BUTTON_A); // Inicializa o pino do Botão A
  gpio_set_dir(BUTTON_A, GPIO_IN); // Define o pino do Botão A como entrada
  gpio_pull_up(BUTTON_A); // Ativa o pull-up interno

  gpio_init(BUTTON_B); // Inicializa o pino do Botão B
  gpio_set_dir(BUTTON_B, GPIO_IN); // Define o pino do Botão B como entrada
  gpio_pull_up(BUTTON_B); // Ativa o pull-up interno
}
// ----------------------------------------------------------------------------------------------------
// Funções de tarefas

// Tarefa para controlar os LEDs
void led_task() {
  // Loop infinito para alternar os LEDs
  while(true){ 
    gpio_put(LED_PIN_BLUE, 0); // Desliga o LED Azul
    gpio_put(LED_PIN_RED, 1); // Liga o LED Vermelho
    vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500 ms

    gpio_put(LED_PIN_RED, 0); // Desliga o LED Vermelho
    gpio_put(LED_PIN_GREEN, 1); // Liga o LED Verde
    vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500 ms

    gpio_put(LED_PIN_GREEN, 0); // Desliga o LED Verde
    gpio_put(LED_PIN_BLUE, 1); // Liga o LED Azul
    vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500 ms
  }
}

// Tarefa para controlar o buzzer
void buzzer_task() {
  // Loop infinito para alternar o buzzer
  while(true){
    pwm_set_gpio_level(BUZZER_PIN, 5000); // Liga o buzzer 

    vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500 ms

    pwm_set_gpio_level(BUZZER_PIN, 0); // Desliga o buzzer

    vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500 ms
  }
}

// Variáveis globais para gerenciar as tarefas
TaskHandle_t led_task_handle = NULL;
TaskHandle_t buzzer_task_handle = NULL;

// Variáveis globais para controlar o estado dos LEDs e do buzzer
bool led_enabled = true;
bool buzzer_enabled = true;

// Tarefa para gerenciar os botões
void buttons_task() {
  // Loop infinito para monitorar os botões
  while(true){
    // Verifica o estado do botão A 
    if (!gpio_get(BUTTON_A)){
      vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
      // Verifica se o LED está habilitado ou não
      if (led_enabled) {
        vTaskSuspend(led_task_handle); // Suspende a tarefa do LED
        led_enabled = false; // Atualiza o estado do LED
      } else {
        vTaskResume(led_task_handle); // Retoma a tarefa do LED
        led_enabled = true; // Atualiza o estado do LED
      }
    }
    // Verifica o estado do botão B
    if (!gpio_get(BUTTON_B)){
      vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
      // Verifica se o buzzer está habilitado ou não
      if (buzzer_enabled) {
        vTaskSuspend(buzzer_task_handle); // Suspende a tarefa do buzzer
        buzzer_enabled = false; // Atualiza o estado do buzzer
        pwm_set_gpio_level(BUZZER_PIN, 0); // Desliga o buzzer
      } else { 
        vTaskResume(buzzer_task_handle); // Retoma a tarefa do buzzer
        buzzer_enabled = true; // Atualiza o estado do buzzer
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // Aguarda 100 ms antes de verificar novamente
  }
}
// ----------------------------------------------------------------------------------------------------

// Função principal
int main() {
  // Inicializa a interface padrão de entrada/saída
  stdio_init_all();

  // Configuração dos periféricos
  setup_buttons();
  setup_leds();
  setup_buzzer();
  
  // Criação das tarefas do FreeRTOS
  xTaskCreate(buttons_task, "BUTTONS_Task", 256, NULL, 1, NULL);
  xTaskCreate(led_task, "LED_Task", 256, NULL, 1, &led_task_handle);
  xTaskCreate(buzzer_task, "BUZZER_Task", 256, NULL, 1, &buzzer_task_handle);

  // Inicia o escalonador do FreeRTOS
  vTaskStartScheduler();

  // Se o escalonador falhar, entra em um loop infinito
  while(1){};
  return 0;
}
