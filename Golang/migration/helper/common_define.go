package helper

const (
	CPP    = "cpp"
	GOLANG = "go"

	KAFKA = "kafka"
	MONGO = "mongo"
	MYSQL = "mysql"
	REDIS = "redis"
	TIDB  = "tidb"

	PREFIX = "/home/godman/Workspace/Practice/Golang/migration/conf/"
	SUFFIX = ".txt"

	LineAttr = "[splines=polyline]"
)

// Relation ...
type Relation struct {
	M map[string]map[string]int
}

// DBRelation ...
type DBRelation struct {
	L string
	D string
	M map[string]map[string]int
}

// DBAttribute ...
var DBAttribute = make(map[string]string)

// LayoutType ...
var LayoutType = []string{"dot", "circo", "fdp"}

func init() {
	DBAttribute[KAFKA] = "[color=red]"
	DBAttribute[MONGO] = "[color=brown]"
	DBAttribute[MYSQL] = "[color=green]"
	DBAttribute[REDIS] = "[color=blue]"
	DBAttribute[TIDB] = "[color=gray]"
}
