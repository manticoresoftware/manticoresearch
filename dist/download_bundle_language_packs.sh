#!/bin/bash
# Download morphology and Jieba language pack files into OUT_DIR.
# Layout matches manticoresearch-language-packs package.yml so files end up
# in the same paths as when installing manticore-language-packs (share/manticore/de, en, ru, jieba).
# Usage: ./dist/download_bundle_language_packs.sh <output_dir>
set -e

OUT_DIR="${1:?Usage: $0 <output_dir>}"
MORPHOLOGY_URL="https://repo.manticoresearch.com/repository/morphology"
CPPJIEBA_DICT="https://raw.githubusercontent.com/manticoresoftware/cppjieba/master/dict"
JIEBA_EXTRA="https://raw.githubusercontent.com/manticoresoftware/jieba/refs/heads/master/extra_dict/dict.txt.big"

mkdir -p "$OUT_DIR"
cd "$OUT_DIR"

# Morphology packs (de, en, ru) - same URLs as language-packs package.yml
for lang in de en ru; do
  echo "Downloading morphology ${lang}.pak.tgz..."
  curl -sSL "${MORPHOLOGY_URL}/${lang}.pak.tgz" | tar -xzf -
done

# Jieba dictionaries - same URLs as language-packs package.yml
mkdir -p jieba
for name in hmm_model idf stop_words user.dict; do
  echo "Downloading jieba ${name}.utf8..."
  curl -sSL "${CPPJIEBA_DICT}/${name}.utf8" -o "jieba/${name}.utf8"
done
echo "Downloading jieba jieba.dict.utf8..."
curl -sSL "$JIEBA_EXTRA" -o jieba/jieba.dict.utf8

echo "Language packs ready in $OUT_DIR"
