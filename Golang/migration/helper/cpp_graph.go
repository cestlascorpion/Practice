package helper

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"sync"
)

// GenerateSvrGraph ...
func GenerateSvrGraph(svr string, relation *Relation, tag bool) error {
	set := make(map[string]int)
	buffer := new(bytes.Buffer)

	buffer.WriteString("digraph G {\n\tnode [shape=box];\n")
	writeSvr2Buffer(svr, relation, buffer, set, 0, tag)
	buffer.WriteString("}")

	var path string
	if tag {
		path = svr + ".svr.tag.dot"
	} else {
		path = svr + ".svr.dot"
	}
	err := Write2DotFile(path, buffer.String())
	if err != nil {
		return err
	}

	var wg sync.WaitGroup
	for _, l := range LayoutType {
		wg.Add(1)
		go func() {
			defer wg.Done()
			var cmd *exec.Cmd
			if tag {
				cmd = exec.Command("dot", "-Tsvg", "-K"+l, "-o", l+"."+svr+".svr.tag.svg", svr+".svr.tag.dot")
			} else {
				cmd = exec.Command("dot", "-Tsvg", "-K"+l, "-o", l+"."+svr+".svr.svg", svr+".svr.dot")
			}
			_, err = cmd.Output()
			if err != nil {
				fmt.Println(err)
			}
		}()
	}
	wg.Wait()
	return nil
}

func writeSvr2Buffer(caller string, relation *Relation, buffer *bytes.Buffer, set map[string]int, depth int, tag bool) {
	_, ok := set[caller]
	if !ok {
		set[caller] = 1
	} else {
		return
	}

	svr, ok := relation.M[caller]
	if !ok {
		return
	}
	for callee := range svr {
		var r string
		if tag {
			r = fmt.Sprintf("\t%s->%s %s;\n", caller+"_"+strconv.Itoa(depth), callee+"_"+strconv.Itoa(depth+1), LineAttr)
		} else {
			r = fmt.Sprintf("\t%s->%s %s;\n", caller, callee, LineAttr)
		}
		buffer.WriteString(r)
		writeSvr2Buffer(callee, relation, buffer, set, depth+1, tag)
	}
}

// Write2DotFile ...
func Write2DotFile(path, content string) error {
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
