# CardioIA — Fase 2: Diagnóstico Automatizado (IA no Estetoscópio Digital)

Nesta fase o CardioIA ganha um módulo que lê relatos de pacientes e ajuda no diagnóstico. São duas partes:

1. **Extração de sintomas e sugestão de diagnóstico:** lê frases de pacientes, identifica os sintomas a partir de um mapa de conhecimento e sugere a doença mais provável.
2. **Classificador de risco:** um modelo de Machine Learning (TF-IDF + Regressão Logística / Árvore de Decisão) que classifica uma frase como **alto risco** ou **baixo risco**, simulando uma triagem clínica.

## Vídeo de demonstração

🎥 **Link (YouTube, não listado):** _adicionar aqui_

## Estrutura

```text
fase2/
├── README.md
├── parte1/
│   ├── frases_sintomas.txt          # 10 relatos de pacientes
│   ├── mapa_conhecimento.csv        # 63 associações sintoma -> doença
│   ├── diagnostico_sintomas.ipynb   # leitura, extração e diagnóstico
│   └── resultado_diagnosticos.csv   # saída gerada pelo notebook
└── parte2/
    ├── frases_risco.csv             # 160 frases rotuladas (alto/baixo risco)
    └── classificador_risco.ipynb    # TF-IDF, treino, avaliação e análise de vieses
```

## Como executar

```bash
pip install pandas scikit-learn matplotlib jupyter
```

Abra os notebooks pelo Jupyter ou VS Code **a partir da pasta de cada parte**, já que os arquivos são lidos por caminho relativo, e execute todas as células.

---

## Parte 1 — Frases de sintomas e mapa de conhecimento

### Relatos (`frases_sintomas.txt`)
São 10 frases escritas como um paciente falaria. Cada uma traz **o que a pessoa sente**, **quando começou** e **como isso afeta a rotina**. Exemplo:

> "Há três semanas sinto uma dor no peito quando caminho rápido, mas ela passa quando paro para descansar, por isso parei de ir a pé para o mercado."

As frases foram pensadas para cobrir doenças diferentes: infarto, angina, insuficiência cardíaca, arritmia, hipertensão, AVC, pericardite, miocardite, trombose/embolia pulmonar e endocardite.

### Mapa de conhecimento (`mapa_conhecimento.csv`)
Usa as colunas `sintoma_1, sintoma_2, doenca_associada`. São 63 linhas e 12 doenças, com sinônimos e variações de escrita ("dor no peito", "aperto no tórax", "pressão no peito"...).

### Como o diagnóstico é feito (`diagnostico_sintomas.ipynb`)
1. O texto é normalizado: minúsculas e sem acentos.
2. Cada expressão do mapa é procurada dentro da frase.
3. Cada sintoma encontrado soma **1 ponto** para a doença. Se os dois sintomas da mesma linha aparecem juntos, a doença ganha **1 ponto extra**.
4. A doença com mais pontos é a sugestão principal, e as próximas aparecem como "outras possibilidades".
5. Uma regex extrai também **quando os sintomas começaram** ("há dois dias", "desde ontem", "hoje de manhã").

### Resultado
O sistema acertou o diagnóstico esperado nas **10 frases**:

| Paciente | Início | Diagnóstico sugerido | Pontos |
|---|---|---|---|
| 1 | há dois dias | Infarto Agudo do Miocárdio | 8 |
| 2 | há uma semana | Insuficiência Cardíaca | 6 |
| 3 | desde ontem | Arritmia | 5 |
| 4 | há três semanas | Angina | 7 |
| 5 | faz uns dez dias | Hipertensão Arterial | 6 |
| 6 | hoje de manhã | AVC | 4 |
| 7 | há quatro dias | Pericardite | 4 |
| 8 | há duas semanas | Miocardite | 5 |
| 9 | há cinco dias | Trombose Venosa Profunda / Embolia Pulmonar | 6 |
| 10 | há um mês | Endocardite Infecciosa | 10 |

**Limitações:** a busca é por correspondência de texto, então sintomas escritos de um jeito muito diferente do mapa não são reconhecidos, e negações ("não sinto dor no peito") não são tratadas. Sintomas genéricos aparecem em várias doenças, por isso mostramos um ranking e não uma resposta única.

---

## Parte 2 — Classificador de risco

### Base (`frases_risco.csv`)
São 160 frases no formato `frase,situacao`, **balanceadas** (80 de alto risco e 80 de baixo risco). Elas cobrem desde sinais de emergência (dor no peito com suor frio, boca torta, desmaio) até queixas comuns (resfriado, dor muscular, cansaço depois de exercício). Algumas frases ambíguas foram incluídas de propósito.

### Pipeline (`classificador_risco.ipynb`)
1. Análise exploratória: distribuição das classes e tamanho das frases.
2. Divisão treino/teste estratificada (75% / 25%).
3. **TF-IDF** com unigramas e bigramas, sem remover stopwords (para manter palavras como "não", "leve" e "pouco").
4. Treino de **Regressão Logística** e **Árvore de Decisão**.
5. Avaliação com acurácia, precision/recall, matriz de confusão e validação cruzada (5 folds).
6. Análise dos termos com mais peso e dos erros.
7. Teste com frases novas, fora da base.

### Resultados

| Modelo | Acurácia (teste) | Validação cruzada (média) | Falsos negativos* |
|---|---|---|---|
| **Regressão Logística** | **87,5%** | **88,1%** | 3 de 20 |
| Árvore de Decisão | 80,0% | 75,6% | 5 de 20 |

\* Frases de alto risco classificadas como baixo risco, o erro mais perigoso numa triagem.

### Vieses e distorções observados
- **O modelo aprende o estilo de escrita:** palavras como "pouco", "leve" e "depois" puxam para baixo risco, e "muito" e "forte" para alto risco. Isso reflete como a base foi escrita, não só os sintomas.
- **Negação:** "não sinto dor no peito nem falta de ar" foi classificada como alto risco (74%).
- **Termos técnicos:** "dispneia" e "taquicardia" não estão na base, então o modelo fica na dúvida (~50%).
- **Overfitting da árvore:** chegou a profundidade 12 para decorar as 120 frases de treino.
- **Origem dos dados:** a base é pequena, foi escrita por um único grupo e não tem idade, sexo ou histórico do paciente.

A análise completa está no final do notebook.

---

## Governança e uso responsável
- Todos os dados desta fase são **simulados**: nenhum dado real de paciente foi usado.
- O mapa de conhecimento e os rótulos de risco foram definidos pelo grupo, sem validação médica.
- Os dois módulos são **ferramentas de apoio** e não substituem a avaliação de um profissional de saúde.
- Erros de classificação na saúde têm impacto real. Por isso o recall da classe "alto risco" e os falsos negativos foram analisados com atenção, e não só a acurácia.

---
**Projeto acadêmico** — FIAP · Método PBL (Project Based Learning).

## Equipe
- Everton Marinho Souza (RM 566767)
- Felipe de Souza Lourenço (RM 567521)
- Matheus Ribeiro Martelletti (RM 566767)
- Júlia Gutierres Fernandes Souza (RM 568296)
