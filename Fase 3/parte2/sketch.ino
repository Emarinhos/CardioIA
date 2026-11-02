#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>

const uint8_t PIN_DHT = 4;
const uint8_t PIN_BOTAO = 15;
const uint8_t SWITCH_PIN = 2;
const uint8_t TIPO_DHT = DHT22;
const unsigned long BAUD = 115200;
const unsigned long INTERVALO_MS = 5000;
const unsigned long DEBOUNCE_MS = 200;
const unsigned long SEM_CLIQUE_MS = 3000;
const uint8_t JANELA = 5;
const unsigned long MIN_BPM = 30;
const unsigned long MAX_BPM = 220;
const unsigned long MS_POR_MINUTO = 60000;
const uint16_t BUFFER_SIZE = 360;
const unsigned long SWITCH_DEBOUNCE_MS = 100;

// Rede virtual do Wokwi: aberta, sem senha; canal 6 conecta mais rapido.
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_SENHA = "";
const int32_t WIFI_CANAL = 6;
const unsigned long WIFI_RETRY_MS = 5000;

// O ESP32 nao tem RTC. O NTP ajusta o relogio quando a rede sobe; depois ele segue sozinho.
const char *NTP_SERVIDOR = "pool.ntp.org";
const long FUSO_SEG = -3 * 3600;
const time_t RELOGIO_VALIDO = 1600000000;

// Broker publico do HiveMQ com TLS. Topico unico porque o broker e compartilhado.
const char *MQTT_HOST = "broker.hivemq.com";
const uint16_t MQTT_PORTA = 8883;
const char *MQTT_TOPICO = "cardioia/566767/esp32/sensores";
const char *MQTT_TOPICO_STATUS = "cardioia/pulseira/status";
const unsigned long MQTT_RETRY_MS = 5000;
const uint16_t MQTT_BUFFER = 512;

DHT dht(PIN_DHT, TIPO_DHT);
WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long ultimaImpressaoMs = 0;
unsigned long ultimoCliqueMs = 0;
bool estavaAlto = true;
unsigned long intervalos[JANELA];
uint8_t nIntervalos = 0;
uint8_t idxIntervalo = 0;
bool leituraInvalida = false;
unsigned long ultimoBpm = 0;

struct Amostra {
  float temperatura;
  float umidade;
  unsigned long bpm;
  unsigned long instanteMs;
  bool invalido;
  // Lida antes do primeiro NTP: instanteMs fica antes da base e nao tem data.
  bool comRelogio;
};

Amostra fila[BUFFER_SIZE];
uint16_t filaInicio = 0;
uint16_t filaQtd = 0;
// Chave ligada = rede permitida. Desligada corta Wi-Fi e MQTT de verdade.
bool redePermitida = false;
bool nivelCandidato = false;
bool candidatoLigado = false;
unsigned long candidatoDesdeMs = 0;
bool redeAssociada = false;
unsigned long ultimaTentativaWifiMs = 0;
bool mqttConectado = false;
unsigned long ultimaTentativaMqttMs = 0;
String mqttClientId;
bool ntpPedido = false;
bool relogioPronto = false;
time_t relogioBaseEpoch = 0;
unsigned long relogioBaseMs = 0;

void iniciarWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA, WIFI_CANAL);
  ultimaTentativaWifiMs = millis();
  Serial.print("Conectando em ");
  Serial.println(WIFI_SSID);
}

void cortarRede() {
  if (mqtt.connected()) {
    mqtt.publish(MQTT_TOPICO_STATUS, "offline", true);
  }
  mqtt.disconnect();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  redeAssociada = false;
  mqttConectado = false;
}

// Sem bloquear o loop: avisa quando associa ou cai e tenta de novo a cada 5s.
void manterWifi(unsigned long agora) {
  if (!redePermitida) {
    return;
  }
  bool associada = WiFi.status() == WL_CONNECTED;
  if (associada != redeAssociada) {
    redeAssociada = associada;
    if (associada) {
      Serial.print("Rede associada, IP ");
      Serial.println(WiFi.localIP());
      configTime(FUSO_SEG, 0, NTP_SERVIDOR);
      ntpPedido = true;
    } else {
      Serial.println("Rede perdida");
    }
  }
  if (associada && ntpPedido && !relogioPronto) {
    time_t agoraRelogio = time(nullptr);
    if (agoraRelogio > RELOGIO_VALIDO) {
      relogioBaseEpoch = agoraRelogio;
      relogioBaseMs = agora;
      relogioPronto = true;
    }
  }
  if (!associada && agora - ultimaTentativaWifiMs >= WIFI_RETRY_MS) {
    ultimaTentativaWifiMs = agora;
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_SENHA, WIFI_CANAL);
  }
}

bool online() {
  return redePermitida && redeAssociada && mqtt.connected();
}

// Estado inicial da chave, antes de qualquer sensor.
void lerChaveInicial() {
  redePermitida = digitalRead(SWITCH_PIN) == HIGH;
}

// DHT22 no GPIO 4; botao no 15; chave no GPIO 2.
void setup() {
  Serial.begin(BAUD);
  pinMode(SWITCH_PIN, INPUT);
  lerChaveInicial();
  dht.begin();
  pinMode(PIN_BOTAO, INPUT_PULLUP);
  // Todo ESP32 do Wokwi tem o mesmo MAC; o sufixo aleatorio evita que o broker derrube sessoes com ID repetido.
  mqttClientId = "cardioia-" + String((uint32_t)ESP.getEfuseMac(), HEX) + "-" + String(esp_random(), HEX);
  // Criptografa sem validar certificado, como na aula; em producao usar a CA do broker.
  wifiClient.setInsecure();
  mqtt.setServer(MQTT_HOST, MQTT_PORTA);
  mqtt.setBufferSize(MQTT_BUFFER);
  if (redePermitida) {
    iniciarWifi();
  } else {
    WiFi.mode(WIFI_OFF);
  }
}

// Fora de 30-220 nao entra na janela; o proximo valido entra.
void registrarClique(unsigned long agora) {
  if (ultimoCliqueMs != 0) {
    unsigned long intervalo = agora - ultimoCliqueMs;
    ultimoBpm = intervalo == 0 ? 0 : MS_POR_MINUTO / intervalo;
    if (ultimoBpm < MIN_BPM || ultimoBpm > MAX_BPM) {
      leituraInvalida = true;
    } else {
      intervalos[idxIntervalo] = intervalo;
      idxIntervalo = (idxIntervalo + 1) % JANELA;
      if (nIntervalos < JANELA) {
        nIntervalos++;
      }
      leituraInvalida = false;
    }
  }
  ultimoCliqueMs = agora;
}

// Media da janela; 0 se ainda nao ha intervalo.
unsigned long bpmAtual() {
  if (nIntervalos == 0) {
    return 0;
  }
  unsigned long soma = 0;
  for (uint8_t i = 0; i < nIntervalos; i++) {
    soma += intervalos[i];
  }
  unsigned long media = soma / nIntervalos;
  if (media == 0) {
    return 0;
  }
  return MS_POR_MINUTO / media;
}

// Descida vale se ja passaram 200ms desde o ultimo clique aceito.
void lerBotao(unsigned long agora) {
  bool alto = digitalRead(PIN_BOTAO) == HIGH;

  if (estavaAlto && !alto && (ultimoCliqueMs == 0 || agora - ultimoCliqueMs >= DEBOUNCE_MS)) {
    registrarClique(agora);
  }
  estavaAlto = alto;

  if (alto && ultimoCliqueMs != 0 && agora - ultimoCliqueMs > SEM_CLIQUE_MS) {
    nIntervalos = 0;
    idxIntervalo = 0;
    ultimoCliqueMs = 0;
    leituraInvalida = false;
    ultimoBpm = 0;
  }
}

// Cheio descarta a amostra mais antiga.
void guardar(const Amostra &a) {
  if (filaQtd == BUFFER_SIZE) {
    fila[filaInicio] = a;
    filaInicio = (filaInicio + 1) % BUFFER_SIZE;
  } else {
    fila[(filaInicio + filaQtd) % BUFFER_SIZE] = a;
    filaQtd++;
  }
}

// NaN do DHT vira null para o JSON continuar valido.
void escreverNumero(char *dst, size_t tam, float v) {
  if (isnan(v)) {
    snprintf(dst, tam, "null");
  } else {
    snprintf(dst, tam, "%.1f", v);
  }
}

bool publicar(const Amostra &a, bool offline) {
  char temp[12];
  char umid[12];
  escreverNumero(temp, sizeof(temp), a.temperatura);
  escreverNumero(umid, sizeof(umid), a.umidade);
  char dataHora[32];
  formatarDataHora(dataHora, sizeof(dataHora), a.instanteMs, a.comRelogio, true);
  char msg[256];
  snprintf(msg, sizeof(msg),
           "{\"temperatura\":%s,\"umidade\":%s,\"bpm\":%lu,\"bpm_invalido\":%s,"
           "\"instante_ms\":%lu,\"offline\":%s,\"data_hora\":\"%s\"}",
           temp, umid, a.bpm, a.invalido ? "true" : "false",
           a.instanteMs, offline ? "true" : "false", dataHora);
  return mqtt.publish(MQTT_TOPICO, msg);
}

// Uma conta só. O serial fica sem fuso; o JSON pede o sufixo -03:00. Sem NTP, "sem-relogio".
void formatarDataHora(char *dst, size_t tam, unsigned long instanteMs, bool comRelogio, bool comFuso) {
  if (!comRelogio || tam == 0) {
    snprintf(dst, tam, "sem-relogio");
    return;
  }
  unsigned long deltaMs = instanteMs - relogioBaseMs;
  time_t t = relogioBaseEpoch + (time_t)(deltaMs / 1000);
  struct tm partes;
  localtime_r(&t, &partes);
  size_t n = strftime(dst, tam, "%Y-%m-%dT%H:%M:%S", &partes);
  if (n == 0 || n >= tam) {
    snprintf(dst, tam, "sem-relogio");
    return;
  }
  if (!comFuso) {
    return;
  }
  long fuso = FUSO_SEG;
  char sinal = '+';
  if (fuso < 0) {
    sinal = '-';
    fuso = -fuso;
  }
  snprintf(dst + n, tam - n, "%c%02d:%02d", sinal, (int)(fuso / 3600), (int)((fuso % 3600) / 60));
}

// Primeiro campo da linha, sempre no mesmo lugar. Sem NTP ainda, o texto fixo "sem-relogio".
void imprimirHora(const Amostra &a) {
  char hora[25];
  formatarDataHora(hora, sizeof(hora), a.instanteMs, a.comRelogio, false);
  Serial.print(hora);
}

void imprimirAmostra(const Amostra &a, bool offline) {
  imprimirHora(a);
  Serial.print(" | ");
  if (offline) {
    Serial.print("offline: ");
  }
  Serial.print("Temp: ");
  Serial.print(a.temperatura, 1);
  Serial.print(" C | Umid: ");
  Serial.print(a.umidade, 1);
  Serial.print(" % | BPM: ");
  if (a.invalido) {
    Serial.print(a.bpm);
    Serial.println(" invalido");
  } else {
    Serial.println(a.bpm);
  }
}

void imprimirCiclo(const Amostra &a, bool enviado) {
  imprimirHora(a);
  Serial.print(" | Temp: ");
  Serial.print(a.temperatura, 1);
  Serial.print(" C | Umid: ");
  Serial.print(a.umidade, 1);
  Serial.print(" % | BPM: ");
  if (a.invalido) {
    Serial.print(a.bpm);
    Serial.print(" invalido");
  } else {
    Serial.print(a.bpm);
  }
  Serial.print(" | Wi-Fi: ");
  Serial.print(redeAssociada ? "conectado" : "desconectado");
  Serial.print(" | MQTT: ");
  Serial.print(mqtt.connected() ? "conectado" : "desconectado");
  Serial.print(" | Envio: ");
  Serial.println(enviado ? "sucesso" : "sem sucesso");
}

// So aceita a chave depois de 100ms no mesmo nivel.
bool chaveEstavel(unsigned long agora, bool &ligado) {
  ligado = digitalRead(SWITCH_PIN) == HIGH;
  if (ligado == redePermitida) {
    nivelCandidato = false;
    return false;
  }
  if (!nivelCandidato || candidatoLigado != ligado) {
    nivelCandidato = true;
    candidatoLigado = ligado;
    candidatoDesdeMs = agora;
    return false;
  }
  if (agora - candidatoDesdeMs < SWITCH_DEBOUNCE_MS) {
    return false;
  }
  nivelCandidato = false;
  return true;
}

// Da mais antiga para a mais nova; so sai da fila o que o broker aceitou.
void enviarLote() {
  if (filaQtd == 0) {
    return;
  }
  uint16_t total = filaQtd;
  while (filaQtd > 0) {
    const Amostra &a = fila[filaInicio];
    if (!publicar(a, true)) {
      break;
    }
    imprimirAmostra(a, true);
    filaInicio = (filaInicio + 1) % BUFFER_SIZE;
    filaQtd--;
  }
  if (filaQtd == 0) {
    filaInicio = 0;
    Serial.print("Lote enviado: sucesso (");
    Serial.print(total);
    Serial.println(" amostras)");
  } else {
    Serial.print("Lote parcial: ");
    Serial.print(total - filaQtd);
    Serial.print(" de ");
    Serial.print(total);
    Serial.println("; o restante fica na fila");
  }
}

// Conecta quando ha rede; a cada conexao confirmada, descarrega a fila.
void manterMqtt(unsigned long agora) {
  if (!redePermitida || !redeAssociada) {
    return;
  }
  if (mqtt.connected()) {
    mqtt.loop();
    return;
  }
  if (mqttConectado) {
    mqttConectado = false;
    Serial.print("MQTT perdido, estado ");
    Serial.println(mqtt.state());
  }
  if (agora - ultimaTentativaMqttMs < MQTT_RETRY_MS && ultimaTentativaMqttMs != 0) {
    return;
  }
  ultimaTentativaMqttMs = agora;
  if (mqtt.connect(mqttClientId.c_str(), MQTT_TOPICO_STATUS, 1, true, "offline")) {
    mqttConectado = true;
    mqtt.publish(MQTT_TOPICO_STATUS, "online", true);
    Serial.print("MQTT conectado em ");
    Serial.println(MQTT_HOST);
    enviarLote();
  } else {
    Serial.print("MQTT falhou, estado ");
    Serial.println(mqtt.state());
  }
}

void aoDesconectar() {
  Serial.println("Wi-Fi desconectado");
  cortarRede();
}

void aoConectar() {
  Serial.println("Wi-Fi conectado");
  ultimaTentativaMqttMs = 0;
  iniciarWifi();
}

void atualizarWifi(unsigned long agora) {
  bool ligado;
  if (!chaveEstavel(agora, ligado)) {
    return;
  }
  redePermitida = ligado;
  if (!ligado) {
    aoDesconectar();
    return;
  }
  aoConectar();
}

// Leitura de temperatura, umidade e BPM neste instante.
Amostra lerAmostra(unsigned long agora) {
  Amostra atual;
  atual.temperatura = dht.readTemperature();
  if (!isnan(atual.temperatura)) {
    if (atual.temperatura < 0) {
      atual.temperatura = 0;
    } else if (atual.temperatura > 50) {
      atual.temperatura = 50;
    }
  }
  atual.umidade = dht.readHumidity();
  atual.instanteMs = agora;
  atual.comRelogio = relogioPronto;
  if (leituraInvalida) {
    atual.bpm = ultimoBpm;
    atual.invalido = true;
  } else if (nIntervalos > 0) {
    atual.bpm = bpmAtual();
    atual.invalido = false;
  } else {
    atual.bpm = 0;
    atual.invalido = false;
  }
  return atual;
}

// Online publica no broker; sem rede ou se o publish falhar, guarda na fila.
void publicarAmostra(const Amostra &a) {
  bool enviado = online() && publicar(a, false);
  imprimirCiclo(a, enviado);
  if (!enviado) {
    guardar(a);
  }
}

void loop() {
  unsigned long agora = millis();
  lerBotao(agora);
  atualizarWifi(agora);
  manterWifi(agora);
  manterMqtt(agora);

  if (agora - ultimaImpressaoMs >= INTERVALO_MS) {
    ultimaImpressaoMs = agora;
    publicarAmostra(lerAmostra(agora));
  }
}
