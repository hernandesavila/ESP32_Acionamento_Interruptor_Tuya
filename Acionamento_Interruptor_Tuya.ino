#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>
#include <time.h>
#include <sys/time.h>

// ***** CONFIGURAÇÕES DE REDE E TUYA ***** //
const char* ssid = "SSID_WIFI";
const char* password = "SENHA_WIFI";

// Credenciais da Tuya (do Developer Platform)
const char* accessKey = "ACCESS_KEY";              // client_id
const char* secretKey = "SECRET";  // secret
const char* deviceId = "DEVICE_ID";

// Variáveis globais para token e expiração (em milissegundos)
String accessToken = "";
unsigned long long tokenExpiration = 0;  // Epoch em milissegundos

// ***** CONFIGURAÇÕES DO BOTÃO ***** //
const int buttonPin = 12;     // Pino onde o botão está conectado
bool deviceState = false;     // Estado do dispositivo: false = desligado, true = ligado
bool lastButtonState = HIGH;  // Assume pull-up: botão não pressionado = HIGH

// ***** FUNÇÕES AUXILIARES ***** //

// Obtém o epoch UTC em milissegundos (utilizando gettimeofday)
long long getTimestamp() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Erro ao obter tempo NTP");
    return 0;
  }

  // Converte para timestamp (epoch) em segundos
  time_t now = mktime(&timeinfo);

  // Converte para milissegundos
  long long timestampMillis = now * 1000LL;  // Converte para milissegundos

  Serial.println("Epoch Time em milissegundos: " + String(timestampMillis));
  return timestampMillis;
}

// Calcula o hash SHA256 de uma string e retorna em formato hexadecimal (minúsculas)
String sha256String(const char* data, size_t len) {
  unsigned char hash[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char*)data, len);
  mbedtls_md_finish(&ctx, hash);
  mbedtls_md_free(&ctx);

  String hashStr = "";
  for (int i = 0; i < 32; i++) {
    if (hash[i] < 16) hashStr += "0";
    hashStr += String(hash[i], HEX);
  }
  return hashStr;  // retorna em minúsculas
}

// ***** FUNÇÃO DE OBTENÇÃO DO TOKEN ***** //
// Fórmula para token:
// stringToSign = HTTPMethod + "\n" + Content-SHA256 + "\n" + Optional_Signature_key + "\n" + URL
// Para token: HTTPMethod = "GET", body vazio
// str = client_id + t + nonce + stringToSign
// sign = HMAC-SHA256(str, secret).toUpperCase()
String computeTokenSignature(String timestamp, String nonce) {
  String httpMethod = "GET";
  String urlPath = "/v1.0/token?grant_type=1";
  String body = "";
  // SHA256 de corpo vazio
  String contentHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  String optionalSignatureKey = "";
  String stringToSign = httpMethod + "\n" + contentHash + "\n" + optionalSignatureKey + "\n" + urlPath;
  String strToSign = String(accessKey) + timestamp + nonce + stringToSign;

  // Calcula HMAC-SHA256 de strToSign usando secretKey
  unsigned char hmac_result[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char*)secretKey, strlen(secretKey));
  mbedtls_md_hmac_update(&ctx, (const unsigned char*)strToSign.c_str(), strToSign.length());
  mbedtls_md_hmac_finish(&ctx, hmac_result);
  mbedtls_md_free(&ctx);

  String sign = "";
  for (int i = 0; i < 32; i++) {
    if (hmac_result[i] < 16) sign += "0";
    sign += String(hmac_result[i], HEX);
  }
  sign.toUpperCase();  // Converte para maiúsculas
  return sign;
}

String getToken() {
  String timestamp = String(getTimestamp());
  String nonce = String(random(10000000, 99999999));
  String sign = computeTokenSignature(timestamp, nonce);

  String url = "https://openapi.tuyaus.com/v1.0/token?grant_type=1";
  HTTPClient http;
  http.begin(url);
  http.addHeader("client_id", accessKey);
  http.addHeader("t", timestamp);
  http.addHeader("nonce", nonce);
  http.addHeader("sign", sign);
  http.addHeader("sign_method", "HMAC-SHA256");

  int httpCode = http.GET();
  String response = http.getString();
  Serial.println("Token Response: " + response);

  String token = "";
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response);
  if (!error && doc["success"] == true) {
    token = doc["result"]["access_token"].as<String>();
    // "expire_time" retorna o período de validade em segundos, ex: 7200
    unsigned long long validity = doc["result"]["expire_time"].as<unsigned long long>();
    // Calcula o momento de expiração: timestamp atual + validade convertida para milissegundos
    tokenExpiration = getTimestamp() + (validity * 1000ULL);

    Serial.println("Expiracao token: " + String(validity));
    Serial.println("Expiracao token convertido: " + String(tokenExpiration));
  } else {
    Serial.println("Erro ao obter token: " + String(doc["msg"].as<const char*>()));
  }
  http.end();
  return token;
}

// ***** FUNÇÃO PARA ENVIAR COMANDO AO DISPOSITIVO ***** //
// Fórmula para comando:
// stringToSign = HTTPMethod + "\n" + Content-SHA256 + "\n" + Optional_Signature_key + "\n" + URL
// Para comando: HTTPMethod = "POST"
// str = client_id + access_token + t + nonce + stringToSign
// sign = HMAC-SHA256(str, secret).toUpperCase()
String computeCommandSignature(String httpMethod, String urlPath, String body, String timestamp, String nonce, String token) {
  String contentHash = (body.length() == 0) ? "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" : sha256String(body.c_str(), body.length());
  String optionalSignatureKey = "";
  String stringToSign = httpMethod + "\n" + contentHash + "\n" + optionalSignatureKey + "\n" + urlPath;
  String strToSign = String(accessKey) + token + timestamp + nonce + stringToSign;

  unsigned char hmac_result[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char*)secretKey, strlen(secretKey));
  mbedtls_md_hmac_update(&ctx, (const unsigned char*)strToSign.c_str(), strToSign.length());
  mbedtls_md_hmac_finish(&ctx, hmac_result);
  mbedtls_md_free(&ctx);

  String sign = "";
  for (int i = 0; i < 32; i++) {
    if (hmac_result[i] < 16) sign += "0";
    sign += String(hmac_result[i], HEX);
  }
  sign.toUpperCase();
  return sign;
}

bool getDeviceStatus() {
    if (getTimestamp() >= tokenExpiration) {
        Serial.println("Token expirado. Obtendo novo token...");
        accessToken = getToken();
    }

    String urlPath = "/v1.0/devices/" + String(deviceId) + "/status";
    String httpMethod = "GET";
    String timestamp = String(getTimestamp());
    String nonce = String(random(10000000, 99999999));
    String sign = computeCommandSignature(httpMethod, urlPath, "", timestamp, nonce, accessToken);

    HTTPClient http;
    String fullUrl = "https://openapi.tuyaus.com" + urlPath;
    http.begin(fullUrl);

    http.addHeader("client_id", accessKey);
    http.addHeader("access_token", accessToken);
    http.addHeader("t", timestamp);
    http.addHeader("nonce", nonce);
    http.addHeader("sign", sign);
    http.addHeader("sign_method", "HMAC-SHA256");

    Serial.println("Enviando comando para: " + fullUrl);
    
    int httpCode = http.GET();
    String response = http.getString();
    Serial.println("Status Response: " + response);
    http.end();

    if (httpCode == 200) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);
        if (doc["success"]) {
            JsonArray statusArray = doc["result"].as<JsonArray>();
            for (JsonObject status : statusArray) {
                if (status["code"] == "switch_1") {
                    return status["value"];
                }
            }
        }
    }
    return false;
}

void sendCommand(bool turnOn) {
  // Antes de enviar o comando, verifica se o token expirou
  if (getTimestamp() >= tokenExpiration) {
    Serial.println("Token expirado. Obtendo novo token...");
    accessToken = getToken();
  }

  String urlPath = "/v1.0/devices/" + String(deviceId) + "/commands";  // Somente o caminho
  String httpMethod = "POST";
  String body = "{\"commands\":[{\"code\":\"switch_1\",\"value\":" + String(turnOn ? "true" : "false") + "}]}";

  String timestamp = String(getTimestamp());
  String nonce = String(random(10000000, 99999999));
  String sign = computeCommandSignature(httpMethod, urlPath, body, timestamp, nonce, accessToken);

  String fullUrl = "https://openapi.tuyaus.com" + urlPath;
  HTTPClient http;
  http.begin(fullUrl);

  http.addHeader("client_id", accessKey);
  http.addHeader("access_token", accessToken);
  http.addHeader("t", timestamp);
  http.addHeader("nonce", nonce);
  http.addHeader("sign", sign);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("Content-Type", "application/json");

  Serial.println("Enviando comando para: " + fullUrl);
  Serial.println("Corpo da requisição: " + body);

  int httpCode = http.POST(body);
  String response = http.getString();
  Serial.println("Command Response (" + String(httpCode) + "): " + response);
  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Sincroniza o horário via NTP
  configTime(0, 0, "pool.ntp.org");
  Serial.println("Sincronizando horário NTP...");
  delay(3000);

  // Obtém o token
  accessToken = getToken();
  Serial.println("Access Token: " + accessToken);
}

void loop() {
  bool buttonState = digitalRead(buttonPin);

  // Se houve transição de não pressionado (HIGH) para pressionado (LOW)
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Aguarda debounce
    delay(50);

    if (accessToken.length() > 0) {
      // Alterna o estado do dispositivo
      deviceState = getDeviceStatus();
      Serial.println("Botão pressionado! Alterando estado para: " + String(!deviceState ? "LIGAR" : "DESLIGAR"));
      sendCommand(!deviceState);
    }
  }

  lastButtonState = buttonState;

  delay(10);  // Pequeno delay para evitar polling excessivo
}
