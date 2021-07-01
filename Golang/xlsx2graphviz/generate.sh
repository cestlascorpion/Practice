#!/bin/zsh

mkdir -p build
cd build || return
rm *.svg
rm *.dot
go build -o xlsx2graphviz ../main.go

cat /tmp/service.list | while read line
do
./xlsx2graphviz -svr=$line -xls=/tmp/services.xlsx -sht=sheet1
dot -Tsvg -o $line.svg $line.dot
dot -Tsvg -o $line.tag.svg $line.tag.dot
done
cd ..