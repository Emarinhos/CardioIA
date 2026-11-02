# CardioIA — Fase 3: Monitoramento Contínuo (IoT no Peito do Paciente)

A Fase 3 coloca o CardioIA no pulso. A ideia é uma pulseira de monitoramento contínuo: o paciente usa um ESP32 que lê temperatura e umidade no DHT22, estima a frequência cardíaca por um botão e decide, no próprio aparelho, se a medida sobe na hora ou espera a rede voltar. Sem Wi-Fi, as amostras ficam numa fila local. Com rede, seguem para a nuvem e aparecem num painel para a equipe acompanhar sinais em tempo real — valor atual, histórico e alerta quando a temperatura ou o ritmo saem do esperado. É edge computing numa aplicação de saúde: a decisão começa no vestível, e a nuvem só recebe o que já foi medido e organizado na borda.

## Projetos no Wokwi

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

## Parte 1 — Armazenamento e processamento local

Pulseira simulada no Wokwi. O firmware e o circuito estão em [parte1/](parte1/); o relato completo, em [parte1/relatorio.md](parte1/relatorio.md).

**Sensores.** O DHT22 liga no GPIO 4, em 3,3 V, e entrega temperatura e umidade a cada ciclo. No enunciado, o DHT22 conta como um sensor. O segundo sinal é o botão no GPIO 15: cada aperto entra no cálculo de BPM (sessenta mil dividido pelo intervalo em milissegundos). Só entram valores entre 30 e 220; a média usa no máximo os cinco últimos intervalos válidos. Sem aperto por três segundos, a conta volta a zero.

**Chave de Wi-Fi.** No GPIO 2, uma chave representa a conexão. Ligada, conectado; desligada, desconectado. No boot, o serial informa o estado. A troca só vale depois de 100 ms no mesmo nível, para evitar falso disparo.

**Fila e resiliência.** Sem rede, a amostra entra numa fila circular de 360 registros em RAM. Na cadência de cinco segundos, isso cobre cerca de trinta minutos offline — margem para elevador, túnel ou queda curta. Se a fila enche, a amostra mais antiga sai e o dado recente tem prioridade. SPIFFS e cartão SD não sobrevivem ao fim da simulação no Wokwi; por isso o armazenamento fica na RAM e o monitor serial faz o papel da nuvem nesta parte. Na reconexão, o lote sai em ordem e a fila é esvaziada, como apagar o arquivo local depois do envio.

**Energia.** Entre leituras o ESP32 entra em light sleep e pode acordar pelo botão no GPIO 15. No hardware real, a varredura de Wi-Fi ficaria espaçada; a chave do simulador não reproduz o rádio.

## Parte 2 — Transmissão para a nuvem e visualização

O mesmo vestível passa a publicar de verdade. O firmware está em [parte2/sketch.ino](parte2/sketch.ino), o fluxo em [parte2/Fluxo_Node_Red.json](parte2/Fluxo_Node_Red.json), o relato em [parte2/relatorio.md](parte2/relatorio.md) e as evidências em [parte2/prints/](parte2/prints/).

**Rede e broker.** Com a chave ligada, o ESP32 associa na rede aberta `Wokwi-GUEST` e publica no broker público da HiveMQ (`broker.hivemq.com`, porta 8883, TLS). Os sensores vão no tópico `cardioia/566767/esp32/sensores`. O estado da pulseira vai em `cardioia/pulseira/status`, com mensagem retida (`online` / `offline`). Sem rede, a coleta continua e a amostra entra na mesma fila de 360; na reconexão, o lote sai da mais antiga para a mais nova, com `"offline": true`.

**Leitura e limites.** A temperatura passa por um clamp de 0 °C a 50 °C antes de entrar na amostra; abaixo de zero vira 0 e acima de 50 vira 50. Se o DHT devolver NaN, o JSON sai com `null` nesse campo. A umidade não entra nesse limite.

**JSON no tópico dos sensores.** Cada publicação leva sete campos:

- `temperatura` — DHT22 em °C (uma casa decimal), já limitada a 0–50; `null` se a leitura falhar
- `umidade` — DHT22 em % (uma casa decimal); `null` se a leitura falhar
- `bpm` — frequência estimada pelo botão (`0` se ainda não há intervalo)
- `bpm_invalido` — `true` se o BPM ficou fora da faixa 30–220
- `instante_ms` — `millis()` no momento da leitura
- `offline` — `true` se a amostra veio da fila; `false` se foi ao vivo
- `data_hora` — ISO 8601 com fuso `-03:00`, ou a string `sem-relogio`

O campo interno `comRelogio` não entra no JSON. Ele só marca se o NTP já estava pronto na leitura e decide o valor de `data_hora`: data formatada ou `sem-relogio`. Assim, uma amostra lida antes do primeiro NTP não recebe data inventada — nem quando sai da fila depois da sincronização.

**Status e testamento MQTT.** No `connect`, o cliente registra um LWT (last will): se a sessão cair sem aviso, o broker publica `offline` no tópico de status. Ao conectar, o firmware publica `online` com retain. Quando a chave desliga a rede, `cortarRede` publica `offline` antes do disconnect e só então corta o Wi-Fi.

**Dashboard no Node-RED.** O painel tem dois grupos lado a lado (Temperatura e BPM) e um grupo Status. Em cada grupo de sinal: medidor com faixa de cor, gráfico de linha e LED de alerta. Temperatura acima de 38 °C e BPM acima de 120 acendem o LED vermelho. O eixo do gráfico usa a hora da medição (`data_hora` ou, se for `sem-relogio`, o `instante_ms`). Online aparece em verde e Offline em vermelho, acompanhando o tópico retido. Grafana Cloud ficou de fora de propósito: o painel do Node-RED já cobre o pedido do enunciado.

**Limitações conhecidas do envio.** O `PubSubClient` publica em QoS 0. Quando `publish()` retorna `true`, isso confirma só a gravação no buffer TCP, não a recepção no broker — a fila não pode garantir que “só sai o que o broker aceitou”. O TLS usa `setInsecure()`: a conexão é criptografada, mas o certificado do broker não é validado. Em produção, o certo seria carregar a CA do HiveMQ.

## Como executar

### Parte 1 — fila e serial no Wokwi

1. Abra o projeto no navegador: [https://wokwi.com/projects/479509150096391169](https://wokwi.com/projects/479509150096391169).
2. Clique em **Start simulation** (ou o play do simulador).
3. Abra o **Serial Monitor** do Wokwi. No boot, o serial informa se o Wi-Fi da chave está ligado ou desligado.
4. Com a chave ligada, a cada cinco segundos aparece uma linha com temperatura, umidade, BPM e “Envio: sucesso”.
5. Desligue a chave: a coleta continua e as amostras entram na fila. Religue: o lote sai em ordem no serial e a fila esvazia.
6. Aperte o botão verde em ritmo constante para ver o BPM subir na linha.
7. (Opcional) Para compilar o firmware localmente, na pasta `Fase 3/parte1/`:

```bash
pio run
```

### Parte 2 — MQTT, HiveMQ e Node-RED

1. Instale as dependências do painel no Node-RED, se ainda não tiver: `node-red-dashboard` e `node-red-contrib-ui-led`.
2. No Node-RED, importe o arquivo [parte2/Fluxo_Node_Red.json](parte2/Fluxo_Node_Red.json) (**Menu → Import → select a file**), faça o deploy e abra o dashboard (em geral `http://127.0.0.1:1880/ui`).
3. Confirme que o nó MQTT do fluxo aponta para `broker.hivemq.com`, porta `8883`, com TLS, nos tópicos `cardioia/566767/esp32/sensores` e `cardioia/pulseira/status`.
4. (Opcional) Compile o firmware na pasta `Fase 3/parte2/`:

```bash
pio run
```

5. Abra o projeto da Parte 2: [https://wokwi.com/projects/479583124125573121](https://wokwi.com/projects/479583124125573121). Se for rodar pelo Cursor/VS Code, abra `parte2/diagram.json` e use **Wokwi: Start Simulator**, com o `wokwi.toml` dessa pasta (o binário fica em `.pio/build/esp32/`).
6. Inicie a simulação com a chave **ligada**. No serial devem aparecer `Conectando em Wokwi-GUEST`, `Rede associada`, `MQTT conectado` e leituras com `Envio: sucesso`.
7. No dashboard: medidores e gráficos de temperatura e BPM, LEDs de alerta acima de 38 °C e 120 bpm, e o status Online em verde.
8. Desligue a chave: o status passa a Offline, as amostras vão para a fila e o serial mostra envio sem sucesso. Religue: o lote sobe no gráfico no horário da medição e o status volta a Online.
9. Aperte o botão para gerar BPM; force temperatura alta no DHT do Wokwi (se disponível) ou aguarde valores de teste para ver o LED vermelho.

Os prints de referência estão em [parte2/prints/](parte2/prints/). Os detalhes de fluxo e resiliência estão em [parte1/relatorio.md](parte1/relatorio.md) e [parte2/relatorio.md](parte2/relatorio.md).

## Equipe

- Everton Marinho Souza (RM 568137)
- Felipe de Souza Lourenço (RM 567521)
- Matheus Ribeiro Martelletti (RM 566767)
- Júlia Gutierres Fernandes Souza (RM 568296)
