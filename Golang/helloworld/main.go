package main

import (
	"errors"
	"fmt"
	"io"
	"math"
	"math/rand"
	"reflect"
	"strings"
	"sync"
	"sync/atomic"
	"time"
	"unsafe"

	"golang.org/x/tour/reader"
	"golang.org/x/tour/tree"
)

var wg sync.WaitGroup

// IPAddr ipv4 address
type IPAddr [4]byte

func (i IPAddr) String() string {
	return fmt.Sprintf("%v.%v.%v.%v", i[0], i[1], i[2], i[3])
}

// ErrNegativeSqrt ...
type ErrNegativeSqrt float64

func (e ErrNegativeSqrt) Error() string {
	return fmt.Sprintf("cannot Sqrt negative number: %f", float64(e))
}

func MySqrt(x float64) (float64, error) {
	if x < 0 {
		return 0, ErrNegativeSqrt(x)
	}
	return math.Sqrt(x), nil
}

func testError() {
	fmt.Println(MySqrt(2))
	fmt.Println(MySqrt(-2))
}

func testString() {
	hosts := map[string]IPAddr{
		"loopback":  {127, 0, 0, 1},
		"googleDNS": {8, 8, 8, 8},
	}
	for name, ip := range hosts {
		fmt.Printf("%s: %v\n", name, ip)
	}
}

func testPointer() {
	v1 := uint(12)
	v2 := int(-13)

	fmt.Println(reflect.TypeOf(v1)) //uint
	fmt.Println(reflect.TypeOf(v2)) //int

	fmt.Println(reflect.TypeOf(&v1)) //*uint
	fmt.Println(reflect.TypeOf(&v2)) //*int

	p := &v1
	p = (*uint)(unsafe.Pointer(&v2)) //使用unsafe.Pointer进行类型的转换

	fmt.Println(reflect.TypeOf(p)) // *unit
	fmt.Println(*p)                // 13
}

func testPool() {
	var count uint32
	pool := sync.Pool{New: func() interface{} {
		return atomic.AddUint32(&count, 1)
	}}

	v1 := pool.Get()
	fmt.Printf("v1 %v\n", v1)
	v2 := pool.Get()
	fmt.Printf("v2 %v\n", v2)

	pool.Put(uint32(100))

	v100 := pool.Get()
	fmt.Printf("v100 %v\n", v100)
}

func testOnce() {
	var count int
	var once sync.Once
	max := rand.Intn(100)
	for i := 0; i < max; i++ {
		once.Do(func() {
			count++
		})
	}
}

func testAtomicValue() {
	var countVal atomic.Value
	countVal.Store([]int{1, 3, 5})

	wg.Add(1)
	go func(countVal *atomic.Value) {
		defer wg.Done()
		countVal.Store([]int{2, 4, 6})
	}(&countVal)
	wg.Wait()

	fmt.Println(countVal)
}

func testChan() {
	intChan := make(chan int, 1)
	ticker := time.NewTicker(time.Second)
	go func() {
		defer close(intChan)
		for range ticker.C {
			select {
			case intChan <- 1:
			case intChan <- 2:
			case intChan <- 3:
			}
		}
		fmt.Println("send end")
	}()

	var sum int
	for e := range intChan {
		fmt.Println("recv", e)
		sum += e
		if sum > 10 {
			fmt.Println("get", sum)
			break
		}
	}
	fmt.Println("recv end")
}

func testIO() {
	r := strings.NewReader("hello, reader")

	// 以每次 8 字节的速度读取它的输出。
	b := make([]byte, 8)
	for {
		n, err := r.Read(b)
		fmt.Printf("n = %v err = %v b = %v\n", n, err, b)
		fmt.Printf("b[:n] = %q\n", b[:n])
		if err == io.EOF {
			break
		}
	}
}

type MyReader struct {
	C byte
}

func NewMyReader(c byte) MyReader {
	return MyReader{c}
}

func (r MyReader) Read(b []byte) (int, error) {
	if b == nil {
		return 0, errors.New("nil buffer")
	}

	for i := 0; i < len(b); i++ {
		b[i] = r.C
	}
	return len(b), nil
}

func testTourReader() {
	reader.Validate(NewMyReader('A'))
}

func Walk(t *tree.Tree, ch chan int) {
	walk(t, ch)
	close(ch)
}

func walk(t *tree.Tree, ch chan int) {
	if t == nil {
		return
	}
	ch <- t.Value
	walk(t.Left, ch)
	walk(t.Right, ch)
}

func Same(t1, t2 *tree.Tree) bool {
	c1 := make(chan int)
	c2 := make(chan int)

	go Walk(t1, c1)
	go Walk(t2, c2)

	for {
		e1, ok1 := <-c1
		e2, ok2 := <-c2
		if ok1 != ok2 {
			return false
		}
		if ok1 && ok2 && e1 != e2 {
			return false
		}
		if !ok1 && !ok2 {
			return true
		}
	}
}

func testTourTree() {
	t1 := tree.New(1)
	t2 := tree.New(2)
	fmt.Println(Same(t1, t1))
	fmt.Println(Same(t1, t2))
}

type T struct {
	A int
	B string
}

func testReflect() {
	var x float64 = 3.2
	v := reflect.ValueOf(x)
	t := reflect.TypeOf(x)
	fmt.Println("value", v, "type", t) // value 3.2 type float64
	var i interface{} = x
	fmt.Println("type", reflect.TypeOf(i))   // type float64
	fmt.Println("value", reflect.ValueOf(i)) // value 3.2

	fmt.Println(v.Kind()) // float64
	fmt.Println(t.Kind()) // float64

	_t := T{100, "101"}
	_s := reflect.ValueOf(&_t).Elem()
	typeOfT := _s.Type()
	for i := 0; i < _s.NumField(); i++ {
		_f := _s.Field(i)
		fmt.Printf("%d: %s %s = %v\n", i, typeOfT.Field(i).Name, _f.Type(), _f.Interface())
	}
}

func main() {
	fmt.Println("hello, world")

	// testAtomicValue()
	// testChan()
	// testPool()
	// testPointer()

	// testString()
	// testError()

	// testIO()
	// testTourReader()

	// testTourTree()

	testReflect()
}
