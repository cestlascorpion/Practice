package helper

import (
	"bytes"
	"fmt"
	"os/exec"
	"sync"
)

// GenerateCppDBGraph ...
func GenerateCppDBGraph(svr string, relation *Relation, usages []*DBRelation) error {
	set := make(map[string]int)
	buffer := new(bytes.Buffer)

	buffer.WriteString("digraph G {\n\tnode [shape=box];\n")
	writeSvr2Buffer(svr, relation, buffer, set, 0, false)
	buffer.WriteString("\n")
	writeDB2Buffer(set, usages, buffer)
	buffer.WriteString("}")

	path := svr + ".db.dot"
	err := Write2DotFile(path, buffer.String())
	if err != nil {
		return err
	}

	var wg sync.WaitGroup
	for _, l := range LayoutType {
		wg.Add(1)
		go func() {
			defer wg.Done()
			cmd := exec.Command("dot", "-Tsvg", "-K"+l, "-o", l+"."+svr+".db.svg", svr+".db.dot")
			_, err = cmd.Output()
			if err != nil {
				fmt.Println(err)
			}
		}()
	}
	wg.Wait()

	return nil
}

func writeDB2Buffer(set map[string]int, usages []*DBRelation, buffer *bytes.Buffer) {
	for svr := range set {
		for _, usage := range usages {
			db, ok := usage.M[svr]
			if ok {
				for addr := range db {
					relation := fmt.Sprintf("\t%s->\"%s\n%s\" %s;\n", svr, usage.D, addr, DBAttribute[usage.D])
					buffer.WriteString(relation)
				}
			}
		}
	}
}
