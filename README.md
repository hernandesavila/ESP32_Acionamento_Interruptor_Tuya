# 🔌 Acionamento de Interruptor Tuya com ESP32

Projeto em **Arduino** para **ESP32** que realiza o acionamento remoto de um interruptor inteligente Tuya via API Cloud. O firmware cuida da autenticação HMAC-SHA256, obtém o token de acesso, consulta o status do dispositivo e alterna o relé pelo pressionamento de um botão físico.

---

## 🛠️ Tecnologias Utilizadas

<p align="center">
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/arduino/arduino-original.svg" alt="Arduino" width="30" height="30"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cplusplus/cplusplus-original.svg" alt="C++" width="30" height="30"/>
  <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/esp32/esp32-original.svg" alt="ESP32" width="30" height="30"/>
</p>

- **ESP32** – microcontrolador principal
- **Arduino Core for ESP32** – ambiente de desenvolvimento e bibliotecas base
- **WiFi.h**, **HTTPClient.h**, **ArduinoJson.h**, **mbedtls** – comunicação com a Tuya Cloud
- **Tuya IoT Platform** – API REST para obtenção de token e envio de comandos

---

## 📂 Estrutura do Projeto

- `Acionamento_Interruptor_Tuya.ino` – sketch principal com toda a lógica de autenticação, consulta de status e envio de comandos à API Tuya
- `LICENSE` – licença MIT do projeto

---

## ✅ Pré-requisitos

- Placa **ESP32** com suporte a Arduino
- **Arduino IDE 2.x** (ou VS Code + PlatformIO) configurado com o core ESP32
- Conta no **Tuya Developer Platform** com `accessKey`, `secretKey` e `deviceId` válidos
- Rede Wi-Fi 2.4 GHz com acesso à internet

---

## ⚙️ Configuração

1. **Credenciais de rede**
   - Altere as constantes `ssid` e `password` para o Wi-Fi local.
2. **Credenciais Tuya**
   - Substitua `accessKey`, `secretKey` e `deviceId` pelas informações do seu projeto Tuya.
3. **Botão físico**
   - Conecte um botão entre o pino **GPIO12** e GND (com `pinMode(INPUT_PULLUP)`, o pino fica em HIGH quando solto).
4. **Servidor NTP**
   - O código usa `configTime(0, 0, "pool.ntp.org")`. Ajuste o fuso horário se necessário.

---

## 🛠️ Compilação

1. Abra o arquivo `Acionamento_Interruptor_Tuya.ino` no **Arduino IDE**.
2. Selecione a placa **ESP32 Dev Module** (ou equivalente) e a porta correta.
3. Instale as bibliotecas necessárias:
   - `ArduinoJson`
   - `WiFi` (já inclusa no core ESP32)
   - `HTTPClient`
4. Faça o upload do sketch para o dispositivo.

---

## ▶️ Execução

1. Conecte o ESP32 à alimentação e ao botão físico.
2. Abra o monitor serial a **115200 bps** para acompanhar logs de autenticação e comandos.
3. Após conectar ao Wi-Fi e sincronizar o horário via NTP, o firmware solicita o token de acesso Tuya.
4. Ao pressionar o botão, o dispositivo consulta o estado atual (`switch_1`) e envia o comando invertendo o valor.
5. As respostas HTTP são exibidas no monitor serial para diagnóstico.

---

## 🔎 Funcionamento

- `getTimestamp()` sincroniza o relógio via NTP e converte para epoch em milissegundos.
- `computeTokenSignature()` e `computeCommandSignature()` geram os hashes HMAC-SHA256 exigidos pela Tuya Cloud.
- `getToken()` solicita e armazena o token, calculando a expiração com base em `expire_time` retornado.
- `getDeviceStatus()` consulta o status do `switch_1` antes de cada comando para garantir o estado atual.
- `sendCommand()` envia o corpo JSON `{ "commands": [{ "code": "switch_1", "value": true|false }] }` para alternar o relé.

---

## 📌 Observações

- Tokens são renovados automaticamente quando `tokenExpiration` é alcançado.
- Certifique-se de que o projeto Tuya esteja configurado para usar o **Data Center US** (`openapi.tuyaus.com`); ajuste a URL se necessário.
- Logs detalhados no monitor serial facilitam o diagnóstico de erros de autenticação ou rede.

---

## 📄 Licença

Este projeto está licenciado sob a [MIT License](LICENSE).
