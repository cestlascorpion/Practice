#!/bin/zsh

mkdir -p build && cd build
rm *.svg
rm *.dot
go build -o migration ../main.go
./migration
