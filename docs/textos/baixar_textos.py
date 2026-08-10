"""
CardioIA - Fase 1, Parte 2 (NLP): download dos textos em dominio publico.

Baixa dois textos do Projeto Gutenberg sobre saude cardiovascular e os salva
nesta mesma pasta (docs/textos/) com nomes padronizados.

Uso:
    python baixar_textos.py
"""

import os
import urllib.request

# id do ebook no Gutenberg -> nome de arquivo local
TEXTOS = {
    "https://www.gutenberg.org/cache/epub/43780/pg43780.txt":
        "lettsomian_lectures_heart_diseases.txt",
    "https://www.gutenberg.org/cache/epub/16230/pg16230.txt":
        "fat_and_blood_mitchell.txt",
}

DESTINO = os.path.dirname(os.path.abspath(__file__))

def baixar():
    for url, nome in TEXTOS.items():
        destino = os.path.join(DESTINO, nome)
        print(f"Baixando {nome} ...")
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
            with urllib.request.urlopen(req, timeout=30) as resp:
                conteudo = resp.read().decode("utf-8", errors="replace")
            with open(destino, "w", encoding="utf-8") as f:
                f.write(conteudo)
            linhas = conteudo.count("\n")
            print(f"  OK -> {destino} ({linhas} linhas)")
        except Exception as e:
            print(f"  ERRO: {e}\n  Baixe manualmente em: {url}")

if __name__ == "__main__":
    baixar()
    print("\nConcluido. Verifique os arquivos em docs/textos/.")
