"""
CardioIA - Fase 1: Gerador de dataset simulado de pacientes cardiacos.

Gera 1000 registros sinteticos com variaveis clinicas plausiveis e correlacoes
realistas (a idade eleva pressao/colesterol; fatores de risco elevam a
probabilidade de doenca cardiaca). NENHUM dado pertence a paciente real.

Reprodutibilidade garantida via seed fixa (governanca de dados).
"""

import numpy as np
import pandas as pd

SEED = 42
rng = np.random.default_rng(SEED)
N = 1000

# ---------------------------------------------------------------------------
# 1. Variaveis demograficas
# ---------------------------------------------------------------------------
idade = rng.integers(29, 78, size=N)                      # 29-77 anos
sexo = rng.choice(["M", "F"], size=N, p=[0.54, 0.46])     # leve predominancia M

# ---------------------------------------------------------------------------
# 2. Variaveis clinicas correlacionadas com a idade
# ---------------------------------------------------------------------------
# Pressao arterial sistolica: cresce ~0.6 mmHg por ano de idade + ruido
pressao_sistolica = (95 + 0.6 * idade + rng.normal(0, 10, N)).round().astype(int)
pressao_sistolica = np.clip(pressao_sistolica, 90, 200)

# Colesterol total: cresce com a idade + ruido
colesterol = (150 + 1.1 * idade + rng.normal(0, 25, N)).round().astype(int)
colesterol = np.clip(colesterol, 120, 400)

# Frequencia cardiaca maxima atingida: DECRESCE com a idade (~220 - idade)
freq_cardiaca_max = (210 - 0.9 * idade + rng.normal(0, 12, N)).round().astype(int)
freq_cardiaca_max = np.clip(freq_cardiaca_max, 70, 202)

# Glicemia de jejum > 120 mg/dL (0/1), mais provavel em idosos
prob_glicemia = np.clip((idade - 40) / 100 + 0.1, 0.05, 0.6)
glicemia_alta = rng.binomial(1, prob_glicemia)

# Fatores de estilo de vida
fumante = rng.choice([0, 1], size=N, p=[0.65, 0.35])
diabetes = np.where(glicemia_alta == 1,
                    rng.choice([0, 1], size=N, p=[0.4, 0.6]),
                    rng.choice([0, 1], size=N, p=[0.9, 0.1]))

# Tipo de dor toracica (categorica clinica classica)
tipo_dor_toracica = rng.choice(
    ["angina_tipica", "angina_atipica", "dor_nao_anginosa", "assintomatico"],
    size=N, p=[0.23, 0.30, 0.27, 0.20])

# ---------------------------------------------------------------------------
# 3. Variavel-alvo: doenca cardiaca (0/1) como funcao dos fatores de risco
# ---------------------------------------------------------------------------
# Score de risco latente -> probabilidade via logistica
score = (
    -1.6  # intercepto: calibra a prevalencia para faixa clinica realista (~35%)
    + 0.045 * (idade - 50)
    + 0.020 * (pressao_sistolica - 130)
    + 0.011 * (colesterol - 200)
    - 0.015 * (freq_cardiaca_max - 150)
    + 0.9 * fumante
    + 1.0 * diabetes
    + 0.6 * (sexo == "M")
    + 0.8 * (tipo_dor_toracica == "angina_tipica")
    + rng.normal(0, 0.7, N)
)
prob_doenca = 1 / (1 + np.exp(-score))
doenca_cardiaca = rng.binomial(1, prob_doenca)

# ---------------------------------------------------------------------------
# 4. Montagem do DataFrame
# ---------------------------------------------------------------------------
df = pd.DataFrame({
    "id_paciente": np.arange(1, N + 1),
    "idade": idade,
    "sexo": sexo,
    "pressao_sistolica_mmhg": pressao_sistolica,
    "colesterol_total_mgdl": colesterol,
    "freq_cardiaca_max_bpm": freq_cardiaca_max,
    "glicemia_jejum_alta": glicemia_alta,
    "diabetes": diabetes,
    "fumante": fumante,
    "tipo_dor_toracica": tipo_dor_toracica,
    "doenca_cardiaca": doenca_cardiaca,
})

# ---------------------------------------------------------------------------
# 5. Injecao controlada de imperfeicoes (para exercitar limpeza de dados)
#    ~1.5% de valores ausentes em colesterol -> ensina tratamento de missing
# ---------------------------------------------------------------------------
idx_missing = rng.choice(N, size=int(0.015 * N), replace=False)
df.loc[idx_missing, "colesterol_total_mgdl"] = np.nan

import os
repo_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
out = os.path.join(repo_dir, "data", "pacientes_cardiacos.csv")
df.to_csv(out, index=False)

print("Dataset gerado:", out)
print("Linhas:", len(df), "| Colunas:", len(df.columns))
print("\nPrevalencia de doenca cardiaca: {:.1%}".format(df.doenca_cardiaca.mean()))
print("Valores ausentes (colesterol):", int(df.colesterol_total_mgdl.isna().sum()))
print("\nPrimeiras linhas:")
print(df.head().to_string(index=False))
print("\nDistribuicao por sexo:")
print(df.sexo.value_counts().to_string())