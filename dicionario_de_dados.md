# Dicionário de Dados — `pacientes_cardiacos.csv`

Documento de governança que descreve cada variável do dataset simulado.
Total: **1000 registros**, **11 colunas**. Nenhum dado pertence a paciente real.

| Coluna | Tipo | Unidade / Valores | Descrição | Relevância clínica |
|---|---|---|---|---|
| `id_paciente` | inteiro | 1–1000 | Identificador único do registro | Rastreabilidade; não é preditor |
| `idade` | inteiro | 29–77 anos | Idade do paciente | Principal fator de risco não modificável |
| `sexo` | categórico | `M` / `F` | Sexo biológico | Risco e apresentação clínica diferem por sexo |
| `pressao_sistolica_mmhg` | inteiro | 90–200 mmHg | Pressão arterial sistólica | Hipertensão é fator de risco maior |
| `colesterol_total_mgdl` | float | 120–400 mg/dL | Colesterol total (contém ~1,5% de ausentes) | Dislipidemia associada à aterosclerose |
| `freq_cardiaca_max_bpm` | inteiro | 70–202 bpm | FC máxima atingida em esforço | Baixa reserva cronotrópica sinaliza risco |
| `glicemia_jejum_alta` | binário | 0 / 1 | Glicemia de jejum > 120 mg/dL | Marcador de risco metabólico |
| `diabetes` | binário | 0 / 1 | Diagnóstico de diabetes | Dobra o risco cardiovascular |
| `fumante` | binário | 0 / 1 | Tabagismo ativo | Fator de risco modificável de alto impacto |
| `tipo_dor_toracica` | categórico | 4 categorias* | Classificação da dor torácica | Angina típica tem alto valor preditivo |
| `doenca_cardiaca` | binário | 0 / 1 | **Variável-alvo**: presença de doença | Rótulo para modelos supervisionados (Fase 2) |

\* `angina_tipica`, `angina_atipica`, `dor_nao_anginosa`, `assintomatico`

## Notas de governança

- **Origem:** dados 100% sintéticos, gerados por `notebooks/gerar_dataset.py` com semente fixa (`SEED=42`), garantindo reprodutibilidade.
- **Ausentes intencionais:** ~1,5% de valores faltantes em `colesterol_total_mgdl`, para exercitar tratamento de dados nas fases seguintes.
- **Correlações embutidas:** a variável-alvo foi derivada de um modelo logístico sobre os fatores de risco, produzindo relações clinicamente plausíveis (idade, tabagismo e diabetes elevam a taxa de doença).
- **Prevalência:** ~40%, dentro de faixa realista para coortes cardiológicas — evitando o viés de classes desbalanceadas.
