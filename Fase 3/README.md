# CardioIA — Fase 3: Monitoramento Contínuo (IoT no Peito do Paciente)

Nesta fase o CardioIA passa a acompanhar sinais no próprio dispositivo. São duas partes:

1. **Armazenamento e processamento local:** pulseira simulada no Wokwi com ESP32. Lê temperatura e umidade, estima a frequência cardíaca e guarda as amostras quando a rede cai.
2. **Transmissão para a nuvem e visualização:** o mesmo vestível publica as medidas por MQTT no HiveMQ e o Node-RED monta o painel em tempo real, com gráfico, medidor e alerta.

## Projeto no Wokwi

**Parte 1:** [https://wokwi.com/projects/479509150096391169](https://wokwi.com/projects/479509150096391169)

**Parte 2:** [https://wokwi.com/projects/479583124125573121](https://wokwi.com/projects/479583124125573121)

## Estrutura

```text
Fase 3/
├── README.md
├── parte1/
│   ├── diagram.json       # ESP32, DHT22, botão e chave
│   ├── sketch.ino         # firmware com fila e serial
│   ├── platformio.ini
│   ├── wokwi.toml
│   └── relatorio.md       # fluxo e lógica de resiliência
└── parte2/
    ├── diagram.json
    ├── sketch.ino         # Wi-Fi real, MQTT e fila
    ├── platformio.ini
    ├── wokwi.toml
    ├── libraries.txt
    ├── Fluxo_Node_Red.json
    ├── prints/            # serial, editor e dashboard
    └── relatorio.md
```

## Como executar

**Parte 1.** Abra o [projeto no Wokwi](https://wokwi.com/projects/479509150096391169) e rode a simulação. A chave liga e desliga o Wi-Fi. O monitor serial mostra cada leitura e o lote guardado quando a rede volta.

**Parte 2.** Compile na pasta da parte, com a extensão PlatformIO:

```bash
pio run
```

Abra o [projeto da Parte 2 no Wokwi](https://wokwi.com/projects/479583124125573121) (ou o `diagram.json` local com a extensão). A chave corta Wi-Fi e MQTT de verdade. Importe [parte2/Fluxo_Node_Red.json](parte2/Fluxo_Node_Red.json) no Node-RED, com os módulos `node-red-dashboard` e `node-red-contrib-ui-led`. O painel assina o broker público da HiveMQ.

O `wokwi.toml` de cada parte aponta o firmware gerado em `.pio/build/esp32/` para o simulador no editor.

## Modelos

Esta fase não treina modelo. O processamento fica no ESP32.

**Parte 1 — borda.** A cada 5 segundos o firmware lê o DHT22 e a frequência cardíaca. O botão não é um eletrodo: cada aperto entra numa janela de até cinco intervalos, só entre 30 e 220 bpm. Fora dessa faixa a leitura fica inválida. Sem aperto por 3 segundos, a conta volta a zero. A chave é o Wi-Fi, com espera de 100 ms antes de aceitar a troca. Conectado, a amostra sai na hora no monitor serial. Desconectado, entra numa fila em RAM de 360 registros. Se a fila enche, a amostra mais antiga sai. Na reconexão, o lote é impresso em ordem e a fila é esvaziada.

**Parte 2 — nuvem.** A leitura e a fila continuam. Online, o ESP32 publica um JSON a cada 5 segundos no tópico `cardioia/566767/esp32/sensores`, no broker `broker.hivemq.com`, porta 8883 com TLS. O estado da pulseira vai em `cardioia/pulseira/status`, com mensagem retida (`online` / `offline`). Cada amostra guarda se o relógio já estava sincronizado no momento da leitura. Se a medida foi feita antes do primeiro NTP, o campo `data_hora` e o serial saem como `sem-relogio`, também quando essa amostra é enviada da fila depois da sincronização. O Node-RED monta o painel: gráfico e medidor de temperatura e de BPM, LED de alerta acima de 38 °C e de 120 bpm, e indicador Online/Offline. Grafana Cloud ficou de fora de propósito: o painel do Node-RED já cobre o pedido do enunciado.

---

## Parte 1 — Armazenamento e processamento local

O circuito está em [parte1/diagram.json](parte1/diagram.json) e o firmware em [parte1/sketch.ino](parte1/sketch.ino). O relato completo do fluxo está em [parte1/relatorio.md](parte1/relatorio.md).

**Temperatura e umidade.** O DHT22 liga no GPIO 4, em 3,3 V. As duas grandezas saem juntas a cada ciclo. No enunciado, o DHT22 conta como um sensor.

**Frequência cardíaca.** O botão no GPIO 15 é o segundo sinal, de livre escolha. O cálculo é sessenta mil dividido pelo tempo, em milissegundos, entre dois apertos aceitos.

**Wi-Fi.** A chave no GPIO 2 representa a conexão. Ligada, conectado. Desligada, desconectado. No boot, o serial informa o estado da chave.

**Resiliência.** O simulador não guarda SPIFFS nem cartão SD de um jeito que sobreviva ao fim da simulação. A fila fica na RAM. Cada amostra vale 5 segundos, então 360 registros cobrem cerca de 30 minutos sem rede: uma queda curta, como elevador ou túnel. O dado recente tem prioridade. Esvaziar a fila depois do envio equivale a apagar o arquivo local. O monitor serial faz o papel da nuvem nesta parte.

**Energia.** Entre leituras o ESP32 entra em light sleep. O botão no GPIO 15 pode acordá-lo. No hardware real, a varredura de Wi-Fi ficaria espaçada, porque a chave do simulador não reproduz o rádio.

### Cenários observados

- **Wi-Fi ligado:** a leitura segue a cada 5 segundos e o serial marca o envio como sucesso.
- **Wi-Fi desconectado:** a coleta continua e as amostras entram na fila.
- **Wi-Fi reconectado:** o lote sai em ordem, o serial confirma o envio e as leituras seguintes voltam a sair direto, sem passar pela fila.

## Parte 2 — Transmissão para a nuvem e visualização

O firmware está em [parte2/sketch.ino](parte2/sketch.ino), o fluxo em [parte2/Fluxo_Node_Red.json](parte2/Fluxo_Node_Red.json) e o relato em [parte2/relatorio.md](parte2/relatorio.md). As evidências do painel e do serial ficam em [parte2/prints/](parte2/prints/).

**Envio.** Com a chave ligada, o ESP32 associa na rede `Wokwi-GUEST` e publica no HiveMQ. Sem rede, a amostra entra na fila de 360. Na reconexão, o lote sai da mais antiga para a mais nova, com `"offline": true`. A temperatura lida passa por um limite de 0 °C a 50 °C antes de entrar na amostra; valor abaixo de zero vira 0 e acima de 50 vira 50. Se o sensor devolver NaN, o JSON sai com `null`.

**Status e testamento MQTT.** No `connect`, o cliente registra um LWT (last will): se a sessão cair sem aviso, o broker publica `offline` em `cardioia/pulseira/status`. Ao conectar, o firmware publica `online` com retain. Quando a chave desliga a rede, `cortarRede` publica `offline` antes do disconnect e só então corta o Wi-Fi.

**JSON no tópico dos sensores.** Cada publicação leva sete campos:

- `temperatura` — DHT22 em °C (uma casa decimal), já limitada a 0–50; `null` se a leitura falhar
- `umidade` — DHT22 em % (uma casa decimal); `null` se a leitura falhar
- `bpm` — frequência estimada pelo botão (`0` se ainda não há intervalo)
- `bpm_invalido` — `true` se o BPM ficou fora da faixa 30–220
- `instante_ms` — `millis()` no momento da leitura
- `offline` — `true` se a amostra veio da fila; `false` se foi ao vivo
- `data_hora` — ISO 8601 com fuso `-03:00`, ou a string `sem-relogio`

O campo interno `comRelogio` não entra no JSON. Ele só marca se o NTP já estava pronto na leitura e decide o valor de `data_hora`: data formatada ou `sem-relogio`.

**Limitações conhecidas do envio.** O `PubSubClient` publica em QoS 0. Quando `publish()` retorna `true`, isso confirma só a gravação no buffer TCP, não a recepção no broker. Por isso a fila não pode garantir que “só sai o que o broker aceitou”. O TLS usa `setInsecure()`: a conexão é criptografada, mas o certificado do broker não é validado. Em produção, o certo seria carregar a CA do HiveMQ.

**Dashboard.** Dois grupos lado a lado (Temperatura e BPM) e um grupo Status. Cada grupo tem medidor com faixa de cor, gráfico de linha e LED de alerta. O eixo do gráfico usa a hora da medição (`data_hora` ou, se for `sem-relogio`, o `instante_ms`).

**Limites do grupo.** Temperatura acima de 38 °C e BPM acima de 120 acendem o LED vermelho. O status Online/Offline acompanha o tópico retido da pulseira (LWT e publicação explícita no corte da chave).

## Governança e uso responsável

- O vestível é simulado. Nenhum sinal veio de um paciente.
- O botão substitui um sensor de pulso. Na Parte 1 a chave só simula a conexão; na Parte 2 ela corta Wi-Fi e MQTT de verdade.
- A fila em RAM se perde ao encerrar a simulação. SPIFFS e cartão microSD ficam para um ESP32 físico.
- O módulo é apoio de monitoramento. Ele não fecha diagnóstico nem dispara conduta clínica.

---

**Projeto acadêmico** — FIAP · Método PBL (Project Based Learning). Visão geral do repositório em [README.md](../README.md).

## Equipe

- Everton Marinho Souza (RM 568137)
- Felipe de Souza Lourenço (RM 567521)
- Matheus Ribeiro Martelletti (RM 566767)
- Júlia Gutierres Fernandes Souza (RM 568296)
