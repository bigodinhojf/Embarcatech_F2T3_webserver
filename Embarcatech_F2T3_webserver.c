// -- Inclusão de bibliotecas
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "pico/cyw43_arch.h"
#include "pico/bootrom.h"

#include "lwip/pbuf.h"           // Lightweight IP stack - manipulação de buffers de pacotes de rede
#include "lwip/tcp.h"            // Lightweight IP stack - fornece funções e estruturas para trabalhar com o protocolo TCP
#include "lwip/netif.h"          // Lightweight IP stack - fornece funções e estruturas para trabalhar com interfaces de rede (netif)

// Credenciais WIFI - Tome cuidado se publicar no github!
#define WIFI_SSID "-"
#define WIFI_PASSWORD "-"

// -- Definição de constantes
// Display I2C
#define display_i2c_port i2c1 // Define a porta I2C
#define display_i2c_sda 14 // Define o pino SDA na GPIO 14
#define display_i2c_scl 15 // Define o pino SCL na GPIO 15
#define display_i2c_endereco 0x3C // Define o endereço do I2C
ssd1306_t ssd; // Inicializa a estrutura do display

// GPIO
#define button_A 5 // Define o Botão A na GPIO 5
#define button_B 6 // Define o Botão B na GPIO 6
#define LED_Green 11 // Define o LED Verde na GPIO 11
#define LED_Blue 12 // Define o LED Azul na GPIO 12
#define LED_Red 13 // Define o LED Vermelho na GPIO 13
#define joystick_Y 26 // VRY do Joystick GPIO 26

// Definição do pino do LED CYW43
#define LED_PIN CYW43_WL_GPIO_LED_PIN   // GPIO do CI CYW43

// Variáveis globais
static volatile uint32_t last_time = 0; // Armazena o tempo do último clique dos botões

// LED RGB
volatile bool LED_activate = false; // Armazena se o LED RGB está ativo
volatile int intensidade = 100; // Guarda o valor da intensidade dos LEDs
volatile int cor_verde = 255; // Guarda o valor de 0 a 255 para o LED Verde
volatile int cor_azul = 255; // Guarda o valor de 0 a 255 para o LED Azul
volatile int cor_vermelho = 255; // Guarda o valor de 0 a 255 para o LED Vermelho

// Temperatura
volatile float temp = 0; // Guarda o valor da temperatura

// Strings para o Display OLED
char str_temp[3]; // Guarda o valor da temperatura em string

// ---------------------------------- Declaração das Funções do Web Server ----------------------------------

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

// Tratamento do request do usuário
void user_request(char **request);

// -------------------------------------- Fim das Funções do Web Server --------------------------------------

// Função para definir a cor e intensidade do LED RGB
void cor_led_rgb(){
    if(LED_activate){
        float level_verde = ((cor_verde/255.0) * intensidade);
        pwm_set_gpio_level(LED_Green, level_verde);
    
        float level_azul = ((cor_azul/255.0) * intensidade);
        pwm_set_gpio_level(LED_Blue, level_azul);
    
        float level_vermelho = ((cor_vermelho/255.0) * intensidade);
        pwm_set_gpio_level(LED_Red, level_vermelho);
    }else{
        pwm_set_gpio_level(LED_Green, 0);
        pwm_set_gpio_level(LED_Blue, 0);
        pwm_set_gpio_level(LED_Red, 0);
    }
}

// Função para fazer a configuração de PWM para GPIO
void pwm_init_gpio(uint gpio, uint wrap){
    gpio_set_function(gpio, GPIO_FUNC_PWM); // Define a função da porta GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(gpio); // Guarda o canal do PWM
    pwm_set_wrap(slice, wrap); // Define o valor do Wrap do canal correspondente
    pwm_set_enabled(slice, true); // Habilita o PWM no canal
}

// Função para definir o valor de temperatura simulado pelo Joystick
void temperatura(){
    adc_select_input(0); // Seleciona o ADC0 referente ao VRY do Joystick (GPIO 26)
    uint16_t value_vry = adc_read(); // Ler o valor do ADC selecionado (ADC0 - VRY) e guarda

    temp = ((value_vry * 35.0) / 4095.0) + 15;
}

// Função para exibir informações no display
void display_oled(){
    // Cria as bordas do display
    ssd1306_rect(&ssd, 0, 0, 127, 63, true, false); // Borda principal
    ssd1306_line(&ssd, 1, 12, 126, 12, true); // Desenha uma linha horizontal
    ssd1306_line(&ssd, 1, 24, 126, 24, true); // Desenha uma linha horizontal

    // Cabeçalho
    ssd1306_draw_string(&ssd, "EMB Web Server", 8, 3); // Desenha uma string
    ssd1306_draw_string(&ssd, "Central da casa", 3, 15); // Desenha uma string

    // Indica se a lampada está acesa
    ssd1306_draw_string(&ssd, "Lampada:   ", 18, 27); // Desenha uma string
    if(LED_activate){
        ssd1306_draw_string(&ssd, "On", 86, 27); // Desenha uma string
    }else{
        ssd1306_draw_string(&ssd, "Off", 82, 27); // Desenha uma string
    }

    // Desenha a barra de intensidade
    ssd1306_draw_string(&ssd, "               ", 3, 38); // Desenha uma string (apaga a parte variável da barra para redesenha-la)
    ssd1306_rect(&ssd, 38, 12, 102, 10, true, false); // Desenha um retângulo
    ssd1306_rect(&ssd, 39, 13, intensidade, 8, true, true); // Desenha um retângulo

    // Temperatura
    ssd1306_draw_string(&ssd, "Temperatura:  C", 3, 51); // Desenha uma string
    sprintf(str_temp, "%.0f", temp); // Converte float em string
    ssd1306_draw_string(&ssd, str_temp, 99, 51); // Desenha uma string

    ssd1306_send_data(&ssd); // Atualiza o display
}

// Função de callback de interrupção dos botões
void gpio_irq_handler(uint gpio, uint32_t events){
    // Debouncing
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Pega o tempo atual e transforma em us
    // 200ms
    if(current_time - last_time > 200000){
        last_time = current_time; // Atualização de tempo do último clique

        if(gpio == button_A){
            LED_activate = !LED_activate;
        }else if(gpio == button_B){
            reset_usb_boot(0, 0);
        }
    }
}

// Função principal
int main()
{
    // -- Inicializações
    // Monitor serial
    stdio_init_all();

    // Display I2C
    i2c_init(display_i2c_port, 400 * 1000); // Inicializa o I2C usando 400kHz
    gpio_set_function(display_i2c_sda, GPIO_FUNC_I2C); // Define o pino SDA (GPIO 14) na função I2C
    gpio_set_function(display_i2c_scl, GPIO_FUNC_I2C); // Define o pino SCL (GPIO 15) na função I2C
    gpio_pull_up(display_i2c_sda); // Ativa o resistor de pull up para o pino SDA (GPIO 14)
    gpio_pull_up(display_i2c_scl); // Ativa o resistor de pull up para o pino SCL (GPIO 15)
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, display_i2c_endereco, display_i2c_port); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd); // Atualiza o display

    // GPIO
    gpio_init(button_A); // Inicia a GPIO 5 do botão A
    gpio_set_dir(button_A, GPIO_IN); // Define a direção da GPIO 5 do botão A como entrada
    gpio_pull_up(button_A); // Habilita o resistor de pull up da GPIO 5 do botão A
    gpio_init(button_B); // Inicia a GPIO 6 do botão Bbutton_B
    gpio_set_dir(button_B, GPIO_IN); // Define a direção da GPIO 6 do botão Bbutton_B como entrada
    gpio_pull_up(button_B); // Habilita o resistor de pull up da GPIO 6 do botão Bbutton_B

    // PWM
    pwm_init_gpio(LED_Green, 100); // Inicia o PWM para a GPIO 11 do LED Verde
    pwm_init_gpio(LED_Blue, 100); // Inicia o PWM para a GPIO 12 do LED Azul
    pwm_init_gpio(LED_Red, 100); // Inicia o PWM para a GPIO 13 do LED Vermelho

    // ADC
    adc_init();
    adc_gpio_init(joystick_Y); // Inicia o ADC para o GPIO 26 do VRY do Joystick

    // Funções de interrupção dos botões A e B
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    //Inicializa a arquitetura do cyw43
    while (cyw43_arch_init())
    {
        printf("Falha ao inicializar Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    // GPIO do CI CYW43 em nível baixo
    cyw43_arch_gpio_put(LED_PIN, 0);

    // Ativa o Wi-Fi no modo Station, de modo a que possam ser feitas ligações a outros pontos de acesso Wi-Fi.
    cyw43_arch_enable_sta_mode();

    // Conectar à rede WiFI - fazer um loop até que esteja conectado
    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        printf("Falha ao conectar ao Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }
    printf("Conectado ao Wi-Fi\n");

    // Caso seja a interface de rede padrão - imprimir o IP do dispositivo.
    if (netif_default)
    {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    // Configura o servidor TCP - cria novos PCBs TCP. É o primeiro passo para estabelecer uma conexão TCP.
    struct tcp_pcb *server = tcp_new();
    if (!server)
    {
        printf("Falha ao criar servidor TCP\n");
        return -1;
    }

    //vincula um PCB (Protocol Control Block) TCP a um endereço IP e porta específicos.
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Falha ao associar servidor TCP à porta 80\n");
        return -1;
    }

    // Coloca um PCB (Protocol Control Block) TCP em modo de escuta, permitindo que ele aceite conexões de entrada.
    server = tcp_listen(server);

    // Define uma função de callback para aceitar conexões TCP de entrada. É um passo importante na configuração de servidores TCP.
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");

    while (true)
    {
        cor_led_rgb();
        temperatura();
        display_oled();

        /* 
        * Efetuar o processamento exigido pelo cyw43_driver ou pela stack TCP/IP.
        * Este método deve ser chamado periodicamente a partir do ciclo principal 
        * quando se utiliza um estilo de sondagem pico_cyw43_arch 
        */
        cyw43_arch_poll(); // Necessário para manter o Wi-Fi ativo
        sleep_ms(100);      // Reduz o uso da CPU
    }

    //Desligar a arquitetura CYW43.
    cyw43_arch_deinit();
    return 0;
}

// -------------------------------------- Funções ---------------------------------

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// Tratamento do request do usuário - digite aqui
void user_request(char **request){

    if (strstr(*request, "GET /led_on") != NULL)
    {
        LED_activate = true;
    }
    else if (strstr(*request, "GET /led_off") != NULL)
    {
        LED_activate = false;
    }
    else if (strstr(*request, "GET /?rgb=") != NULL) {
        char hex[8] = {0};  // Ex: "C84646"
        int intensidade_val;

        // Tenta extrair os valores com uma regex simples (ignora o %23)
        if (sscanf(*request, "GET /?rgb=%%23%6[0-9A-Fa-f]&i=%d", hex, &intensidade_val) == 2) {
            int r, g, b;
            if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
                cor_vermelho = r;
                cor_verde = g;
                cor_azul = b;
                intensidade = intensidade_val;

                printf("RGB extraído: R=%d G=%d B=%d Intensidade=%d\n", r, g, b, intensidade_val);
            }
        }
    }
    printf("Led: %d | Vermelho: %.d | Verde: %.d | Azul: %.d | Intensidade: %.d", LED_activate, cor_vermelho, cor_verde, cor_azul, intensidade);
};

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Alocação do request na memória dinámica
    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    // Tratamento de request - Controle dos LEDs
    user_request(&request);

    // Cria a resposta HTML
    char html[1024];

    // Converter o valor RGB para Hex e mostrar no HTML
    char color_hex[8];  // Ex: "#C84646"
    snprintf(color_hex, sizeof(color_hex), "#%02X%02X%02X", cor_vermelho, cor_verde, cor_azul);

    // Instruções html do webserver
    snprintf(html, sizeof(html), // Formatar uma string e armazená-la em um buffer de caracteres
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "<!DOCTYPE html>\n"
             "<html>\n"
             "<head>\n"
             "<title>Minha casa</title>\n"
             "<style>\n"
             "body{background-color:#6E0000;font-family:Arial,sans-serif;text-align:center;margin-top:50px;color:#fff;}\n"
             "h1{font-size:64px;margin-bottom:30px;}\n"
             "h2{font-size:48px;margin:10px;}\n"
             "label{font-size:24px;margin:30px 10px}\n"
             "button{background-color:#C84646;font-size:24px;margin:10px;padding:15px 30px;border-radius:10px;color:#fff;}\n"
             ".c{display: flex;justify-content:center;align-items: center;}\n"
             "</style>\n"
             "</head>\n"
             "<body>\n"
             "<h1>Central da casa</h1>\n"
             "<h2>Lampada</h2>\n"
             "<div class=\"c\">\n"
             "<form action=\"./led_on\"><button>On</button></form>\n"
             "<form action=\"./led_off\"><button>Off</button></form><br>\n"
             "</div>\n"
             "<form action=\"/\" method=\"GET\">\n"
             "<label>Cor RGB:</label>\n"
             "<input type=\"color\" name=\"rgb\"value=\"%s\"><br>\n"
             "<label>Intensidade:</label>\n"
             "<input type=\"range\" name=\"i\" min=\"0\" max=\"100\" value=\"%d\"><br>\n"
             "<button type=\"submit\">Aplicar</button>\n"
             "<h2>Temperatura: %.1f &deg;C</h2>\n"
             "</form>\n"
             "</body>\n"
             "</html>\n",
            color_hex, intensidade, temp);

    // Escreve dados para envio (mas não os envia imediatamente).
    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);

    // Envia a mensagem
    tcp_output(tpcb);

    //libera memória alocada dinamicamente
    free(request);
    
    //libera um buffer de pacote (pbuf) que foi alocado anteriormente
    pbuf_free(p);

    return ERR_OK;
}
