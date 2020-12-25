# Blinding

## helloworld

试验田。

## xlsx2graphviz

xlsx文件要求：第一列为服务名称，第二列为依赖的服务名（使用一个空格分割）。

懒人一键生成。

```shell
sudo apt-get update
sudo apt-get install graphviz

cd binding
go mod tidy
go mod download
go mod vendor

cd xlsx2graphviz/generate
./generate.sh
```

单独使用xlsx2graphviz命令生成dot文件。

```shell
./xlsx2graphviz -svr=服务名 -xls=文档路径 -sht=表名
```
