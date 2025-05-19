# Aluno: Wilton Lacerda Silva Júnior
## Matrícula: TIC370100193
# Video explicativo: https://youtu.be/enEqImDKBH4
# Alerta de Enchentes
O objetivo do projeto é desenvolver utilizando a placa BitDogLab um modo de monitorar chuvas e enchentes de cidades. Ela simulará na entrada do joystick valores de chuva e de nível de água, e mostrará um aviso dependendo desses valores.
## Funcionalidades

- **Display OLED**
  - O display mostrará o monitoramento das do eixo X e do eixo Y do joystick, sendo respectivamente as variáveis de Chuva e a de Água. Também mostrará uma mensagem caso os níveis das variáveis cheguem em alarmantes.
- **1 Botão**
  - O botão B está sendo utilizado com a função de reiniciar a placa no modo de bootload.
- **MATRIZ DE LEDs**
   - A matriz de LEDs ajudará como saída visual para o caso de alerta. Ele também servirá para mostrar caso o caso de alertá não estiver.
- **BUZZER**
  - O buzzer servirá como saída sonora para o caso de alerta.
- **LED RGB**
   - O LED RGB do meio da placa também servirá, assim como a matriz de LED, para mostrar caso esteja ou não em estado de alerta.
- **JOYSTICK**
  - O Joystick servirá para simular a entrada dos dados da chuva e da água.

# Requisitos
## Hardware:

- Raspberry Pi Pico W.
- 1 display ssd1306 com o sda na porta 14 e o scl na porta 15.
- 1 matriz de led 5x5 na porta 7.
- 1 led rgb, com o led vermelho no pino 13 e o led verde no pino 11.
- 1 botão no pino 5.
- 1 buzzer no pino 10.
- 2 potenciômetros, no pino 26 e 27.

## Software:

- Ambiente de desenvolvimento VS Code com extensão Pico SDK.

# Instruções de uso
## Configure o ambiente:
- Certifique-se de que o Pico SDK está instalado e configurado no VS Code.
- Configure o FreeRTOS para a configuração da sua máquina.
- Compile o código utilizando a extensão do Pico SDK.
## Teste:
- Utilize a placa BitDogLab para o teste. Caso não tenha, conecte os hardwares informados acima nos pinos correspondentes.

# Explicação do projeto:
## Contém:
- O projeto terá 1 meio de entrada: O joystick.
- Também contará com saídas visuais, sendo a matriz de led, o led rgb no meio da placa, o buzzer, e o display OLED.

## Funcionalidades:
- O programa mostrará representação de modo de alerta na matriz de led, buzzer, LED central e no display OLED.
- O programa mostrará os dados obtidos no display OLED.
- O Joystick servirá como modo de simular dados para testar o monitoramento.
