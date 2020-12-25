package main

import (
	"bytes"
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/360EntSecGroup-Skylar/excelize/v2"
)

var svr string
var xls string
var sht string

func init() {
	flag.StringVar(&svr, "svr", "", "service name")
	flag.StringVar(&xls, "xls", "/home/godman/Source/Blinding/xlsx2graphviz/generate/services.xlsx", "xls's path")
	flag.StringVar(&sht, "sht", "one", "sheet name")
	flag.Parse()
}

func main() {
	if len(svr) == 0 || len(xls) == 0 || len(sht) == 0 {
		fmt.Println("invalid parameters")
		return
	}

	src, err := excelize.OpenFile(xls)
	if err != nil {
		fmt.Println(xls, err)
		return
	}

	rows, err := src.GetRows(sht)
	if err != nil {
		fmt.Println(xls, err)
		return
	}

	dict := make(map[string][]string)
	for _, row := range rows {
		if len(row) != 2 {
			continue
		}
		ss := strings.Split(row[1], " ")
		for _, s := range ss {
			s = strings.TrimSpace(s)
			if len(s) != 0 && s != row[0] {
				if dict[row[0]] == nil {
					dict[row[0]] = make([]string, 0, len(ss))
				}
				dict[row[0]] = append(dict[row[0]], s)
			}
		}
	}

	set := make(map[string]int) // 去重
	buffer := new(bytes.Buffer) // dot

	buffer.WriteString("digraph G {\n")
	dump(svr, dict, buffer, set, 0) // 递归添加节点关系
	buffer.WriteString("}")

	err = writeDot(svr+".dot", buffer.String())
	if err != nil {
		fmt.Println(err)
	}
}

func dump(caller string, dict map[string][]string, buffer *bytes.Buffer, set map[string]int, depth int) {
	_, ok := set[caller]
	if !ok {
		set[caller] = 1
	} else {
		return
	}

	ss, ok := dict[caller]
	if !ok {
		return
	}
	for _, callee := range ss {
		relation := fmt.Sprintf("\t%s->%s;\n", caller+"_"+strconv.Itoa(depth), callee+"_"+strconv.Itoa(depth+1))
		buffer.WriteString(relation)
		dump(callee, dict, buffer, set, depth+1)

	}
}

func writeDot(path, content string) error {
	f, err := os.OpenFile(path, os.O_RDWR|os.O_CREATE|os.O_APPEND, 0644)
	if err != nil {
		return err
	}
	if _, err := f.Write([]byte(content)); err != nil {
		return err
	}
	if err := f.Close(); err != nil {
		return err
	}
	return nil
}
