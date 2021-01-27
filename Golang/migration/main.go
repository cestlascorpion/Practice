package main

import (
	"fmt"
	dot "github.com/emicklei/dot"
	helper "golang/migration/helper"
	"os/exec"
)

const (
	relation = "/home/godman/Workspace/Practice/Golang/migration/conf/relation.txt"
)

func main() {
	drawWithDot()
	drawWithHelper()
}

func drawWithDot() {
	r, err := helper.ParseRelation(relation)
	if err != nil {
		fmt.Println(err)
		return
	}

	for s, _ := range r.M {
		sub, err := helper.SplitRelation(r, s)
		if err != nil {
			fmt.Println(err)
		}

		if len(sub.M) == 0 {
			continue
		}

		graph := dot.NewGraph(dot.Directed)
		nodes := addNode(graph, sub)
		addLine(graph, sub, nodes)
		err = helper.Write2DotFile(s+".svr.dot", graph.String())
		if err != nil {
			fmt.Println(err)
		}

		cmd := exec.Command("dot", "-Tsvg", "-o", s+".svr.svg", s+".svr.dot")
		_, err = cmd.Output()
		if err != nil {
			fmt.Println(err)
		}
	}
}

func addNode(graph *dot.Graph, relation *helper.Relation) map[string]dot.Node {
	nodes := make(map[string]int)
	result := make(map[string]dot.Node)
	for key, val := range relation.M {
		nodes[key] += 1
		for svr := range val {
			nodes[svr] += 1
		}
	}
	for svr := range nodes {
		node := graph.Node(svr).Box().Attr("style", "filled")
		result[svr] = node
	}
	return result
}

func addLine(graph *dot.Graph, relation *helper.Relation, nodes map[string]dot.Node) {
	for key, val := range relation.M {
		for svr := range val {
			graph.Edge(nodes[key], nodes[svr], "").Attr("splines", "line").Attr("color", "black")
		}
	}
}

func drawWithHelper() {
	r, err := helper.ParseRelation(relation)
	if err != nil {
		fmt.Println(err)
		return
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
