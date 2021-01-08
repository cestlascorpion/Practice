#!/bin/zsh

mkdir -p build && cd build
rm *.svg
rm *.dot
go build -o xlsx2graphviz ../main.go

cat ../../../Conf/service.list | while read line
do
./xlsx2graphviz -svr=$line -xls=../../../Conf/services.xlsx -sht=one
dot -Tsvg -o $line.svg $line.dot
dot -Tsvg -o $line.tag.svg $line.tag.dot
done

cd ..