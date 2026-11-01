# CardioIA — Fase 3: Monitoramento Contínuo (IoT no Peito do Paciente)

Nesta fase o CardioIA passa a acompanhar sinais no próprio dispositivo. São duas partes:

1. **Armazenamento e processamento local:** pulseira simulada no Wokwi com ESP32. Lê temperatura e umidade, estima a frequência cardíaca e guarda as amostras quando a rede cai.
2. **Transmissão para a nuvem e visualização:** envio por MQTT e dashboard em tempo real. Essa parte ainda não está na pasta.

## Projeto no Wokwi

**Parte 1:** [https://wokwi.com/projects/479509150096391169](https://wokwi.com/projects/479509150096391169)

## Estrutura

```text
Fase 3/
├── README.md
└── parte1/
    ├── diagram.json       # ESP32, DHT22, botão e chave
    ├── sketch.ino         # firmware da pulseira
    ├── platformio.ini
    ├── wokwi.toml
    └── relatorio.md       # fluxo e lógica de resiliência
```

## Como executar

Abra o [projeto no Wokwi](https://wokwi.com/projects/479509150096391169) e rode a simulação. A chave liga e desliga o Wi-Fi. O monitor serial mostra cada leitura e o lote guardado quando a rede volta.

Para compilar na pasta da parte, com a extensão PlatformIO:

```bash
pio run
```

O `wokwi.toml` aponta o firmware gerado em `.pio/build/esp32/` para o simulador no editor.

## Modelos

Esta fase não treina modelo. O processamento fica no ESP32.

**Parte 1 — borda.** A cada 5 segundos o firmware lê o DHT22 e a frequência cardíaca. O botão não é um eletrodo: cada aperto entra numa janela de até cinco intervalos, só entre 30 e 220 bpm. Fora dessa faixa a leitura fica inválida. Sem aperto por 3 segundos, a conta volta a zero. A chave é o Wi-Fi, com espera de 100 ms antes de aceitar a troca. Conectado, a amostra sai na hora no monitor serial. Desconectado, entra numa fila em RAM de 360 registros. Se a fila enche, a amostra mais antiga sai. Na reconexão, o lote é impresso em ordem e a fila é esvaziada.

**Parte 2 — nuvem.** O enunciado pede MQTT (por exemplo HiveMQ Cloud) e um dashboard no Node-RED, com gráfico, medidor e alerta. Esse fluxo ainda não está implementado nesta pasta.

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

O enunciado pede um ESP32 enviando os sinais por MQTT, um dashboard no Node-RED com gráfico de um sinal vital, medidor de outro parâmetro e alerta quando os limites do grupo forem ultrapassados (por exemplo, frequência acima de 120 bpm ou temperatura acima de 38 °C). Grafana Cloud é opcional. Código, prints e relatório dessa parte ainda não estão no repositório.

## Interpretação dos resultados

**Parte 1.** O protótipo mostra a decisão na borda: o dispositivo lê, guarda e só descarrega quando a chave indica rede. A fila de 360 amostras é uma escolha de projeto para falhas curtas, não o teto do ESP32. Os 360 registros ocupam pouco da RAM do chip. Perder a simulação apaga a fila, porque ela não está em arquivo. O botão aproxima o ritmo. Ele não mede pulso. O serial confirma o desenho da resiliência. Ele não é um broker na nuvem.

**Parte 2.** Ainda não há métrica nem tela para interpretar.

---

## Governança e uso responsável

- O vestível é simulado. Nenhum sinal veio de um paciente.
- O botão substitui um sensor de pulso. A chave substitui a conexão Wi-Fi.
- A fila em RAM se perde ao encerrar a simulação. SPIFFS e cartão microSD ficam para um ESP32 físico.
- O módulo é apoio de monitoramento. Ele não fecha diagnóstico nem dispara conduta clínica.

---

**Projeto acadêmico** — FIAP · Método PBL (Project Based Learning). Visão geral do repositório em [README.md](../README.md).

## Equipe

- Everton Marinho Souza (RM 568137)
- Felipe de Souza Lourenço (RM 567521)
- Matheus Ribeiro Martelletti (RM 566767)
- Júlia Gutierres Fernandes Souza (RM 568296)
