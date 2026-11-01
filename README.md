# FIAP - Faculdade de Informática e Administração Paulista

# PROJETO CARDIOIA

---

## 👥 Integrantes:

- Everton Marinho Souza (RM 568137)
- Felipe de Souza Lourenço (RM 567521)
- Matheus Ribeiro Martelletti (RM 566767)
- Júlia Gutierres Fernandes Souza (RM 568296)

---

## 👩‍🏫 Professores:

### Tutor(a)

- Leonardo Ruiz Orabona

### Coordenador(a)

- Andre Godoy, PhD

---

## 📜 Descrição:

O **CardioIA** é um projeto acadêmico do curso de **Inteligência Artificial (FIAP)** com foco em **cardiologia**. Doenças cardiovasculares são a principal causa de mortes no mundo, com cerca de 17,9 milhões de óbitos anuais. Infartos, arritmias, insuficiência cardíaca e AVCs são frequentes e, em muitos casos, podem ser prevenidos com diagnóstico precoce. A proposta é uma plataforma digital de apoio à decisão que simula um ecossistema de cardiologia moderna: organizar dados clínicos, ler relatos de pacientes, monitorar sinais, interpretar exames de imagem, acompanhar o paciente em casa e antecipar picos de risco.

O repositório reúne o trabalho do grupo em **sete fases** sequenciais. As **Fases 1 e 2** já estão entregues. A Fase 1 constrói a base (dados numéricos simulados, corpus textual em domínio público e amostra de raio-X), com origem, limitações e governança explícitas. A Fase 2 lê relatos em português: um módulo de regras sugere um diagnóstico a partir dos sintomas, e classificadores supervisionados (TF-IDF com Regressão Logística e Árvore de Decisão) separam a frase em **alto risco** ou **baixo risco**. Os módulos sugerem e priorizam. Funcionam como apoio à avaliação de um profissional de saúde.

Em termos técnicos, o material entregue até aqui usa **Python**, **pandas**, **NLTK** (estemizador RSLP) e **scikit-learn**. As fases seguintes preveem IoT com ESP32, visão computacional, chatbot, séries temporais e a integração da plataforma.

---

## 🗂 ESTRUTURA GERAL

```text
CardioIA/
│
├── Fase 1/                              # Fundação de dados
│   ├── data/
│   │   └── pacientes_cardiacos.csv      # 1000 pacientes simulados
│   ├── docs/
│   │   ├── dicionario_de_dados.md
│   │   └── textos/                      # corpus em domínio público
│   ├── notebooks/
│   │   └── gerar_dataset.py             # geração reprodutível do CSV
│   ├── assets/
│   │   └── imagens_raiox/               # subconjunto de raio-X de tórax
│   └── README.md
│
├── Fase 2/                              # Diagnóstico automatizado
│   ├── parte1/                          # sintomas, mapa e diagnóstico
│   │   ├── frases_sintomas.txt
│   │   ├── mapa_conhecimento.csv
│   │   ├── diagnostico_sintomas.ipynb
│   │   └── resultado_diagnosticos.csv
│   ├── parte2/                          # classificador de risco
│   │   ├── frases_risco.csv
│   │   └── classificador_risco.ipynb
│   └── README.md
│
├── requirements.txt                     # dependências para reproduzir os experimentos
└── README.md                            # este arquivo
```

---

## Fases do projeto

Cada fase entregue tem **README próprio** com dados, modelos, interpretação dos resultados e limitações. Instalação e execução ficam neste arquivo. As fases 3 a 7 constam do plano do projeto e ainda serão entregues.

| Fase | Tema (resumo) | README |
|------|----------------|--------|
| **1** | Batimentos de Dados — Mapeando o Coração Moderno: base de pacientes cardiológicos, fontes públicas e simuladas, governança dos dados | [Fase 1/README.md](<Fase 1/README.md>) |
| **2** | Diagnóstico Automatizado — IA no Estetoscópio Digital: extração de sintomas e classificadores supervisionados de risco | [Fase 2/README.md](<Fase 2/README.md>) · [▶ Vídeo](https://www.youtube.com/watch?v=78A2zGsYbNo) |
| **3** | Monitoramento Contínuo — IoT no Peito do Paciente: wearable com ESP32 e dashboard de sinais em tempo real | A entregar |
| **4** | Coração em Imagens — Diagnóstico com Visão Computacional: interpretar exames e destacar alterações suspeitas | A entregar |
| **5** | Suporte Digital ao Paciente — Assistente Cardiológico Virtual: chatbot de acompanhamento, com NLP, ética e empatia | A entregar |
| **6** | Coração Sob Controle — Previsão de Crises com IA: séries temporais para picos de risco e protocolos de emergência | A entregar |
| **7** | CardioIA — Plataforma de Inteligência Cardíaca Total: integração dos módulos, usabilidade e arquitetura final | A entregar |

---

## Instalação

É preciso ter **Python 3.11 ou superior**. Na raiz do repositório:

```bash
python -m pip install -r requirements.txt
```

O arquivo [requirements.txt](requirements.txt) fixa as versões usadas na entrega (pandas, NumPy, Matplotlib, scikit-learn, SciPy, NLTK e Jupyter), para o mesmo experimento poder ser repetido em outro ambiente.

Na primeira execução da Parte 1 da Fase 2, o notebook baixa o recurso `rslp` do NLTK, usado pelo estemizador de português.

---

## Execução

**Fase 1 — regerar o dataset numérico.** O script grava `Fase 1/data/pacientes_cardiacos.csv` com a semente `SEED=42`. Na raiz do repositório:

```bash
python "Fase 1/notebooks/gerar_dataset.py"
```

**Fase 2 — notebooks.** Abra cada notebook a partir da pasta da parte correspondente. Os arquivos são lidos por caminho relativo. Execute todas as células.

```bash
# Parte 1 — extração de sintomas e sugestão de diagnóstico
# abrir Fase 2/parte1/diagnostico_sintomas.ipynb

# Parte 2 — classificador de risco
# abrir Fase 2/parte2/classificador_risco.ipynb
```

No Jupyter, a partir da raiz:

```bash
jupyter notebook "Fase 2/parte1/diagnostico_sintomas.ipynb"
jupyter notebook "Fase 2/parte2/classificador_risco.ipynb"
```

Se o notebook for aberto pelo Jupyter a partir da raiz, o diretório de trabalho pode deixar de ser a pasta da parte e a leitura dos CSVs falha. Nesse caso, execute o notebook pelo VS Code ou pelo Jupyter com o diretório de trabalho em `Fase 2/parte1` ou `Fase 2/parte2`.

Modelos e interpretação dos resultados estão no README de cada fase: [Fase 1](<Fase 1/README.md>) e [Fase 2](<Fase 2/README.md>).

---

**Projeto acadêmico** — FIAP · Método PBL (Project Based Learning).

---

🧾 Licença

 MODELO GIT FIAP por Fiap está licenciado sobre Attribution 4.0 International.
