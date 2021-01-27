package main

import (
	"fmt"
	helper "golang/migration/helper"
)

const (
	relation = "/home/godman/Workspace/Practice/Golang/migration/conf/relation.txt"
)

func main() {
	r, err := helper.ParseRelation(relation)
	if err != nil {
		fmt.Println(err)
		return
	}

	for svr := range r.M {
		err := helper.GenerateSvrGraph(svr, r, false)
		if err != nil {
			fmt.Println(svr, err)
			return
		}
	}

	usages, err := helper.GenerateDBUsages(helper.CPP)
	if err != nil {
		fmt.Println(err)
		return
	}

	for svr := range r.M {
		err := helper.GenerateCppDBGraph(svr, r, usages)
		if err != nil {
			fmt.Println(err)
			return
		}
	}
}
