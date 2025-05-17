<div align="center">
    <img src="https://moodle.embarcatech.cepedi.org.br/pluginfile.php/1/theme_moove/logo/1733422525/Group%20658.png" alt="Logo Embarcatech" height="100">
</div>

<br>

# Web Server - Automação Residencial

## Sumário

- [Descrição](#descrição)
- [Funcionalidades Implementadas](#funcionalidades-implementadas)
- [Ferramentas utilizadas](#ferramentas-utilizadas)
- [Objetivos](#objetivos)
- [Instruções de uso](#instruções-de-uso)
- [Vídeo de apresentação](#vídeo-de-apresentação)
- [Aluno e desenvolvedor do projeto](#aluno-e-desenvolvedor-do-projeto)
- [Licensa](#licença)

## Descrição

Este projeto implementa um sistema interativo utilizando a placa de desenvolvimento BitDogLab, o microcontrolador RP2040 e o módulo WI-FI CYW43439, integrando botão, LED RGB, Joystick, Display OLED e página Web.
A partir da página Web o usuário pode controlar o estado da lâmpada inteligente (simulada pelo LED RGB), sua cor e intensidade, e verificar a temperatura da casa. Na placa BitDogLab o botão A muda também o estado da lâmpada, simulando um interruptor comum, o eixo Y do Joystick simula um sensor de temperatura e o Display OLED simula um painel de controle, onde é exibido as informações do estado da lâmpada e o valor da temperatura.

## Funcionalidades Implementadas

1. Lâmpada Inteligente (LED RGB)

   - O LED RGB foi implementado utilizando PWM, para ser possível controlar sua cor e intensidade.
   - Foi desenvolvida uma fórmula que transforma os valores de cor e intensidade para cada LED, para a escala do PWM:

<div align="center">
    <img width = "60%" src = "">
</div>

   - O usuário modifica esses valores de cor e intensidade a partir de inputs na página Web.
   - O usuário mofica o estado do LED por botões na página Web e pelo botão A.

2. Sensor de Temperatura (Joystick)

   - O valor de temperatura é simulado pela movimentação do eixo Y do Joystick.
   - Foi desenvolvida uma fórmula que transforma os valores ADC lidos do eixo Y do Joystick para a escala de temperatura definida (15°C a 50°C):

<div align="center">
    <img width = "60%" src = "">
</div>

3. Página Web

   - O usuário tem controle do estado da lâmpada, sua cor e intensidade.
   - O usuário pode observar a temperatura da casa.
   - A troca de informações entre o microcontrolador e a página Web é feita a partir de requisições.

4. Display OLED

   - É exibido no Display OLED as informações do estado da lâmpada e sua intensidade e o valor da temperatura.
  
## Ferramentas utilizadas

- **Ferramenta educacional BitDogLab (versão 6.3)**: Placa de desenvolvimento utilizada para programar o microcontrolador e o módulo WI-FI.
- **Microcontrolador Raspberry Pi Pico W**: Responsável pelo controle dos periféricos e do módulo WI-FI.
- **Módulo WI-FI CYW43439**: Responsável por se conectar a rede local e fazer a comunicação entre a página Web e o microcontrolador.
- **Joystick**: Utilizado para simular o sensor de temperatura.
- **Display OLED SSD1306**: Exibe as informações da lâmpada e de temperatura.
- **Botão A**: Utilizado para alterar o estado do LED RGB.
- **LED RGB**: Simula uma lâmpada inteligente que liga, desliga, muda de cor e intensidade.
- **Visual Studio Code (VS Code)**: IDE utilizada para o desenvolvimento do código com integração ao Pico SDK.
- **Pico SDK**: Kit de desenvolvimento de software utilizado para programar o Raspberry Pi Pico W em linguagem C.

## Objetivos

1. Aplicar os conceitos aprendidos sobre conexão local WI-FI e implementar o uso do módulo WI-FI.
2. Aplicar a configuração de um servidor HTTP (Web Server).
3. Implementar um sistema interativo entre uma página Web e a placa de desenvolvimento BitDogLab.

## Instruções de uso

1. **Clonar o Repositório**:

```bash
git clone https://github.com/bigodinhojf/Embarcatech_F2T3_webserver.git
```

2. **Compilar e Carregar o Código**:
   No VS Code, configure o ambiente e compile o projeto com os comandos:

```bash	
cmake -G Ninja ..
ninja
```

3. **Interação com o Sistema**:
   - Conecte a placa ao computador.
   - Coloque as credenciais da sua rede local nas variáveis "WIFI_SSID" e "WIFI_PASSWORD".
   - Clique em run usando a extensão do raspberry pi pico.
   - Abra o monitor serial e acompanhe a conexão WI-FI, caso ocorra erro clique no botão reset da placa e tente a conexão novamente.
   - Caso ocorra uma conexão bem sucedida, copie o endereço IP mostrado no monitor serial e o acesse em um dispositivo móvel conectado a mesma rede.
   - Interaja com a página WEB: Ligue e desligue a lâmpada, mude a cor e intensidade, observe a temperatura atual.
   - Na BitDogLab, pressione o botão A para ligar e desligar o LED, mova o eixo Y do Joysticck para alterar a temperatura.
   - Observe as informações exibidas no Display OLED.


## Vídeo de apresentação

O vídeo apresentando o projeto pode ser assistido [clicando aqui](https://youtu.be/xQ59yjjeT0o).

## Aluno e desenvolvedor do projeto

<a href="https://github.com/bigodinhojf">
        <img src="https://github.com/bigodinhojf.png" width="150px;" alt="João Felipe"/><br>
        <sub>
          <b>João Felipe</b>
        </sub>
</a>

## Licença

Este projeto está licenciado sob a licença MIT.
