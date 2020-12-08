#!/bin/sh

path=$1
resultspath=$2
shift 2

mkdir -p "$resultspath"
rm -f "$resultspath/results.txt"

files=$(find "$path" -name "*.txt")

for file in $files;
do
    #filename=$(basename "$file")
    mkdir -p $(dirname "$resultspath/$file")
    ./ae_project.exe -in $file -out "$resultspath/$file" "$@" | tee -a "$resultspath/results.txt"
    #echo ./ae_project.exe -in $file -out "$resultspath/$file" "$@" tee -a "$resultspath/results.txt"
done