# CardioIA — Fase 1: Batimentos de Dados

Repositório da Fase 1 do projeto CardioIA, cujo objetivo é levantar, organizar e documentar as três bases de dados fundamentais que alimentarão os módulos inteligentes das fases seguintes: dados numéricos (para modelos de risco), dados textuais (para NLP) e dados visuais (para Visão Computacional).

Esta fase não treina modelos de IA. Ela constrói e documenta a fundação de dados — com atenção explícita à governança de dados e ao viés.

## Estrutura do repositório
```text
CardioIA_Fase1_Dados/
├── README.md                 # este arquivo
├── data/
│   └── pacientes_cardiacos.csv # dataset numérico (1000 pacientes)
├── docs/
│   ├── dicionario_de_dados.md # descrição de cada variável
│   └── textos/                # arquivos .txt para NLP (Parte 2)
├── notebooks/
│   └── gerar_dataset.py       # script reprodutível do dataset
└── assets/                    # imagens de apoio / amostras (Parte 3)
```

## Parte 1 — Dados Numéricos (IoT / dados tabulares)

**Arquivo:** [data/pacientes_cardiacos.csv](data/pacientes_cardiacos.csv) — 1000 registros, 11 variáveis.

**Origem:** dados simulados (sintéticos), gerados pelo script [notebooks/gerar_dataset.py](notebooks/gerar_dataset.py) com semente fixa para reprodutibilidade. Optou-se por dados simulados porque dados clínicos reais são protegidos por questões éticas, legais e de privacidade — uma restrição comum e legítima em projetos de IA na saúde. O gerador embute correlações clinicamente plausíveis (idade, tabagismo e diabetes elevam a taxa de doença), o que torna a base útil para o aprendizado de padrões reais nas fases seguintes.

**Link público para o dataset completo:** [https://drive.google.com/drive/folders/1O_84GUkaW6_6iOFTlTy0y07eaLDM1v9s?usp=sharing](https://drive.google.com/drive/folders/1O_84GUkaW6_6iOFTlTy0y07eaLDM1v9s?usp=sharing) 

### Variáveis mais relevantes do ponto de vista clínico
- **idade** — principal fator de risco não modificável; risco cardiovascular cresce de forma acentuada com a idade.
- **pressão sistólica** — hipertensão é um dos maiores fatores de risco para eventos cardíacos.
- **colesterol total** — dislipidemia está diretamente ligada à aterosclerose.
- **diabetes / glicemia** — diabéticos têm aproximadamente o dobro do risco cardiovascular.
- **tabagismo** — fator de risco modificável de altíssimo impacto.
- **tipo de dor torácica** — angina típica tem forte valor preditivo para doença coronariana.
- **doença_cardiaca** — variável-alvo (rótulo) para os classificadores supervisionados da Fase 2.

O detalhamento completo de cada coluna está em [docs/dicionario_de_dados.md](docs/dicionario_de_dados.md).

### Governança e viés
- **Reprodutibilidade:** semente fixa (SEED=42) garante que qualquer pessoa regenere o dataset idêntico.
- **Prevalência calibrada (~40%):** evita o viés de classes desbalanceadas que prejudicaria os classificadores.
- **Balanceamento por sexo:** ~55% M / ~45% F, evitando sub-representação de um grupo.
- **Valores ausentes controlados (~1,5% no colesterol):** simulam a imperfeição de dados reais e exercitam o tratamento de missing data.

## Parte 2 — Dados Textuais (NLP)

**Arquivos:** dois textos em domínio público sobre saúde cardiovascular, na pasta [docs/textos/](docs/textos/):
- `lettsomian_lectures_heart_diseases.txt` — The Lettsomian Lectures on Diseases and Disorders of the Heart and Arteries in Middle and Advanced Age, de J. Mitchell Bruce. Tratado clínico sobre causas, sintomas, diagnóstico, prognóstico e tratamento de doenças cardiovasculares.
- `fat_and_blood_mitchell.txt` — Fat and Blood, de S. Weir Mitchell. Aborda doença cardíaca, compensação, sintomas e abordagens de tratamento e recuperação.

**Origem e governança:** ambos obtidos do Projeto Gutenberg, em domínio público — o que elimina restrições de direitos autorais para uso, redistribuição e processamento, uma escolha de governança que garante reprodutibilidade e conformidade legal do corpus.
- Texto 1: https://www.gutenberg.org/ebooks/43780
- Texto 2: https://www.gutenberg.org/ebooks/16230

### Como esses textos poderão ser explorados por algoritmos de NLP
- **Extração de entidades e sintomas (NER):** identificar automaticamente termos clínicos — sintomas (dispneia, angina, palpitações), condições (endocardite, valvopatia) e tratamentos — construindo um vocabulário médico estruturado a partir de texto livre.
- **Classificação de tópicos:** segmentar trechos por assunto (diagnóstico, prognóstico, terapêutica), útil para organizar e recuperar informação clínica.
- **Análise de sentimento / tom clínico:** avaliar a carga de gravidade ou incerteza na descrição de casos, exercício-base para, futuramente, interpretar relatos de pacientes no assistente virtual (Fase 5).
- **Modelagem de linguagem de domínio:** os textos servem como corpus para ajustar vocabulário e embeddings ao jargão cardiológico, melhorando tarefas posteriores.

### Por que essas análises são relevantes para IA na saúde
Grande parte da informação clínica vive em texto não estruturado (prontuários, laudos, literatura). Ensinar algoritmos a ler, estruturar e interpretar esse material permite triagem automática, apoio à decisão e recuperação de evidências — capacidades centrais de uma plataforma como o CardioIA. Começar por um corpus público e bem delimitado é a forma responsável de desenvolver e validar essas técnicas antes de aplicá-las a dados sensíveis de pacientes reais.

> **Nota:** por serem obras históricas (início do séc. XX), esses textos têm valor metodológico (treinar e validar pipelines de NLP), não como fonte de conduta clínica atual — uma distinção importante de governança e de consciência de viés temporal dos dados.

## Parte 3 — Dados Visuais (Visão Computacional)

**Exame escolhido:** raio-X de tórax com foco em cardiomegalia (aumento da área cardíaca) — o raio-X de tórax é o principal método de imagem para identificar o coração aumentado, o que o torna um exame genuinamente cardiológico.

**Fonte:** conjunto de 100+ imagens (.jpg/.png) extraídas do NIH Chest X-ray Dataset, base pública e desidentificada com 112.120 radiografias rotuladas em 14 categorias de achados, entre elas Cardiomegaly.
- Dataset completo: https://www.kaggle.com/datasets/nih-chest-xrays/data
- Amostra reduzida (mais leve para download): https://www.kaggle.com/datasets/nih-chest-xrays/sample
- Versão redimensionada 224×224: https://www.kaggle.com/datasets/khanfashee/nih-chest-x-ray-14-224x224-resized

**Link público para o subconjunto de imagens usado neste projeto:** [https://drive.google.com/drive/folders/1O_84GUkaW6_6iOFTlTy0y07eaLDM1v9s?usp=sharing](https://drive.google.com/drive/folders/1O_84GUkaW6_6iOFTlTy0y07eaLDM1v9s?usp=sharing) (garanta acesso "qualquer pessoa com o link" para a correção da FIAP).

### Como essas imagens poderão ser analisadas por Visão Computacional
- **Detecção de padrões e classificação:** redes neurais convolucionais (CNNs) podem aprender a distinguir radiografias com e sem cardiomegalia — abordagem já consolidada com arquiteturas como ResNet, DenseNet e VGG16 via transfer learning.
- **Identificação de bordas e segmentação:** delinear o contorno do coração e da caixa torácica permite calcular o índice cardiotorácico (razão entre a largura do coração e a do tórax), a medida clássica usada para diagnosticar cardiomegalia.
- **Reconhecimento de anomalias:** localizar regiões suspeitas na imagem (mapas de atenção) destaca onde o modelo "olha" para decidir, aumentando a interpretabilidade.

### Por que essas análises são relevantes para IA na saúde
O raio-X de tórax é um dos exames mais frequentes e de baixo custo, mas sua leitura é desafiadora e depende de especialista. Algoritmos de Visão Computacional podem apoiar a triagem, priorizar casos graves e ampliar o acesso ao diagnóstico em regiões com poucos radiologistas — o objetivo do módulo de diagnóstico por imagem do CardioIA (Fase 4).

### Governança e viés (pontos de atenção)
- **Privacidade:** as imagens do NIH são desidentificadas na origem, adequadas para uso acadêmico e livre.
- **Rótulos imperfeitos:** os rótulos foram extraídos por NLP dos laudos, com acurácia estimada acima de 90% — ou seja, contêm ruído, o que deve ser considerado.
- **Classes desbalanceadas:** a cardiomegalia representa uma fração pequena do total de imagens; ao montar o subconjunto, é importante balancear casos positivos e negativos para não enviesar o modelo.

## Como regerar o dataset numérico
```bash
pip install numpy pandas
python notebooks/gerar_dataset.py
```

---
**Projeto acadêmico** — FIAP · Método PBL (Project Based Learning).

## Equipe
- Everton Marinho Souza (RM 566767)
- Felipe de Souza Lourenço (RM 567521)
- Matheus Ribeiro Martelletti (RM 566767)
- Júlia Gutierres Fernandes Souza (RM 568296)
