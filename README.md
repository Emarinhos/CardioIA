# CardioIA — Fase 1: Batimentos de Dados

Repositório da **Fase 1** do projeto CardioIA, cujo objetivo é levantar, organizar e
documentar as três bases de dados fundamentais que alimentarão os módulos inteligentes
das fases seguintes: **dados numéricos** (para modelos de risco), **dados textuais**
(para NLP) e **dados visuais** (para Visão Computacional).

Esta fase não treina modelos de IA. Ela constrói e documenta a fundação de dados —
com atenção explícita à **governança de dados** e ao **viés**.

---

## Estrutura do repositório

```
CardioIA_Fase1_Dados/
├── README.md                     # este arquivo
├── data/
│   └── pacientes_cardiacos.csv   # dataset numérico (1000 pacientes)
├── docs/
│   ├── dicionario_de_dados.md    # descrição de cada variável
│   └── textos/                   # arquivos .txt para NLP (Parte 2)
├── notebooks/
│   └── gerar_dataset.py          # script reprodutível do dataset
└── assets/                       # imagens de apoio / amostras (Parte 3)
```

---

## Parte 1 — Dados Numéricos (IoT / dados tabulares)

**Arquivo:** [`data/pacientes_cardiacos.csv`](data/pacientes_cardiacos.csv) — 1000 registros, 11 variáveis.

**Origem:** dados **simulados** (sintéticos), gerados pelo script
[`notebooks/gerar_dataset.py`](notebooks/gerar_dataset.py) com semente fixa para
reprodutibilidade. Optou-se por dados simulados porque dados clínicos reais são
protegidos por questões éticas, legais e de privacidade — uma restrição comum e
legítima em projetos de IA na saúde. O gerador embute **correlações clinicamente
plausíveis** (idade, tabagismo e diabetes elevam a taxa de doença), o que torna a
base útil para o aprendizado de padrões reais nas fases seguintes.

> Link público para o dataset completo: **[INSERIR LINK DO GOOGLE DRIVE / ONEDRIVE]**
> _(garanta acesso "qualquer pessoa com o link" para a correção da FIAP)._

### Variáveis mais relevantes do ponto de vista clínico

- **idade** — principal fator de risco não modificável; risco cardiovascular cresce de forma acentuada com a idade.
- **pressão sistólica** — hipertensão é um dos maiores fatores de risco para eventos cardíacos.
- **colesterol total** — dislipidemia está diretamente ligada à aterosclerose.
- **diabetes / glicemia** — diabéticos têm aproximadamente o dobro do risco cardiovascular.
- **tabagismo** — fator de risco modificável de altíssimo impacto.
- **tipo de dor torácica** — angina típica tem forte valor preditivo para doença coronariana.
- **doença_cardiaca** — variável-alvo (rótulo) para os classificadores supervisionados da Fase 2.

O detalhamento completo de cada coluna está em
[`docs/dicionario_de_dados.md`](docs/dicionario_de_dados.md).

### Governança e viés

- **Reprodutibilidade:** semente fixa (`SEED=42`) garante que qualquer pessoa regenere o dataset idêntico.
- **Prevalência calibrada (~40%):** evita o viés de classes desbalanceadas que prejudicaria os classificadores.
- **Balanceamento por sexo:** ~55% M / ~45% F, evitando sub-representação de um grupo.
- **Valores ausentes controlados (~1,5% no colesterol):** simulam a imperfeição de dados reais e exercitam o tratamento de _missing data_.

---

## Parte 2 — Dados Textuais (NLP)

_A preencher: no mínimo dois arquivos `.txt` sobre saúde cardiovascular, na pasta
`docs/textos/`, com fontes como SciELO, BVS, SUS ou Projeto Gutenberg._

**Como esses textos poderão ser explorados por NLP:**
extração de sintomas, análise de sentimentos em relatos de pacientes, e
classificação de tópicos — capacidades que alimentarão o assistente virtual (Fase 5).

---

## Parte 3 — Dados Visuais (Visão Computacional)

_A preencher: no mínimo 100 imagens (`.jpg`/`.png`) de um exame cardiológico
(ECG, angiograma ou raio-X torácico)._

> Link público para as imagens: **[INSERIR LINK DO GOOGLE DRIVE / ONEDRIVE]**

**Como essas imagens poderão ser analisadas por Visão Computacional:**
detecção de padrões, identificação de bordas e reconhecimento de anomalias —
base para o módulo de diagnóstico por imagem (Fase 4).

---

## Como regerar o dataset numérico

```bash
pip install numpy pandas
python notebooks/gerar_dataset.py
```

---

_Projeto acadêmico — FIAP · Método PBL (Project Based Learning)._
