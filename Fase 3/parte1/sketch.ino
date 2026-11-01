#include <DHT.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

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

DHT dht(PIN_DHT, TIPO_DHT);

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
};

Amostra fila[BUFFER_SIZE];
uint16_t filaInicio = 0;
uint16_t filaQtd = 0;
bool wifiConectado = false;
bool nivelCandidato = false;
bool candidatoLigado = false;
unsigned long candidatoDesdeMs = 0;

// Estado inicial da chave, antes de qualquer sensor.
void anunciarWifi() {
  wifiConectado = digitalRead(SWITCH_PIN) == HIGH;
  Serial.println(wifiConectado ? "Wi-Fi conectado" : "Wi-Fi desconectado");
}

// DHT22 no GPIO 4; botao no 15; chave no GPIO 2.
void setup() {
  Serial.begin(BAUD);
  pinMode(SWITCH_PIN, INPUT);
  anunciarWifi();
  dht.begin();
  pinMode(PIN_BOTAO, INPUT_PULLUP);
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

void imprimirAmostra(const Amostra &a, bool offline) {
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

// Em ordem, da mais antiga para a mais nova.
void despejar() {
  for (uint16_t i = 0; i < filaQtd; i++) {
    imprimirAmostra(fila[(filaInicio + i) % BUFFER_SIZE], true);
  }
  filaInicio = 0;
  filaQtd = 0;
}

void imprimirCiclo(const Amostra &a) {
  Serial.print("Temp: ");
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
  Serial.print(wifiConectado ? "conectado" : "desconectado");
  Serial.print(" | Envio: ");
  Serial.println(wifiConectado ? "sucesso" : "sem sucesso");
}

// So aceita a chave depois de 100ms no mesmo nivel.
bool chaveEstavel(unsigned long agora, bool &ligado) {
  ligado = digitalRead(SWITCH_PIN) == HIGH;
  if (ligado == wifiConectado) {
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

void aoDesconectar() {
  Serial.println("Wi-Fi desconectado");
}

void enviarLote() {
  if (filaQtd == 0) {
    return;
  }
  despejar();
  Serial.println("Lote enviado: sucesso");
}

void aoConectar() {
  Serial.println("Wi-Fi conectado");
  enviarLote();
}

void atualizarWifi(unsigned long agora) {
  bool ligado;
  if (!chaveEstavel(agora, ligado)) {
    return;
  }
  wifiConectado = ligado;
  if (!ligado) {
    aoDesconectar();
    return;
  }
  aoConectar();
}

// Light sleep curto; o botao no GPIO 15 acorda na hora.
void dormirCurto(unsigned long agora) {
  unsigned long falta = INTERVALO_MS - (agora - ultimaImpressaoMs);
  if (falta < 15 || falta > INTERVALO_MS) {
    return;
  }
  uint64_t us = (uint64_t)min(falta, 50UL) * 1000ULL;
  rtc_gpio_pullup_en((gpio_num_t)PIN_BOTAO);
  rtc_gpio_pulldown_dis((gpio_num_t)PIN_BOTAO);
  esp_sleep_enable_timer_wakeup(us);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BOTAO, 0);
  esp_light_sleep_start();
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}

// Leitura de temperatura, umidade e BPM neste instante.
Amostra lerAmostra(unsigned long agora) {
  Amostra atual;
  atual.temperatura = dht.readTemperature();
  atual.umidade = dht.readHumidity();
  atual.instanteMs = agora;
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

// Imprime a linha; sem rede tambem guarda.
void publicarAmostra(const Amostra &a) {
  imprimirCiclo(a);
  if (!wifiConectado) {
    guardar(a);
  }
}

void loop() {
  unsigned long agora = millis();
  lerBotao(agora);
  atualizarWifi(agora);

  if (agora - ultimaImpressaoMs >= INTERVALO_MS) {
    ultimaImpressaoMs = agora;
    publicarAmostra(lerAmostra(agora));
    return;
  }
  dormirCurto(millis());
}
