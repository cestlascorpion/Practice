package helper

import (
	"bytes"
	"fmt"
	"os/exec"
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
	err := write2DotFile(path, buffer.String())
	if err != nil {
		return err
	}

	for _, l := range LayoutType {
		cmd := exec.Command("dot", "-Tsvg", "-K"+l, "-o", l+"."+svr+".db.svg", svr+".db.dot")
		_, err = cmd.Output()
		if err != nil {
			return err
		}
	}

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
