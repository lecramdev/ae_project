#!/bin/sh

path=$1
resultsfile=$2
resultspath=$3
shift 3

mkdir -p "$resultspath"
rm -f "$resultsfile"

for file in "$path"/*.txt;
do
    filename=$(basename "$file")
    ./ae_project.exe -in $file -out "$resultspath/$filename" "$@" >> "$resultsfile"
    #echo ./ae_project.exe -in $file -out "result/$filename" "$@"
done