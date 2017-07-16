package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	xj "github.com/basgys/goxml2json"
)

func main() {
	var err error
	var data []byte
	var sData string
	buf := &bytes.Buffer{}
	var buf2 bytes.Buffer

	if data, err = ioutil.ReadFile("eqemu_config.xml"); err != nil {
		fmt.Println("Failed to open eqemu_config.xml:", err.Error())
		os.Exit(1)
	}

	//detect malformed xml in eqemuconfig
	sData = strings.Replace(string(data), "<?xml version=\"1.0\">", "<?xml version=\"1.0\"?>", 1)
	r := strings.NewReader(sData)
	dec := xj.NewDecoder(r)
	root := &xj.Node{}
	if err = dec.DecodeWithCustomPrefixes(root, "", ""); err != nil {
		fmt.Println("Failed to decode eqemu_config.xml:", err.Error())
		os.Exit(1)
	}

	enc := xj.NewEncoder(buf)
	err = enc.EncodeWithCustomPrefixes(root, "", "")
	if err != nil {
		fmt.Println("Failed to encode eqemu_config.xml:", err.Error())
		os.Exit(1)
	}

	//prettyprint
	if err = json.Indent(&buf2, buf.Bytes(), "", "\t"); err != nil {
		fmt.Println("Failed to encode json:", err.Error())
		os.Exit(1)
	}

	if err = ioutil.WriteFile("eqemu_config.json", buf2.Bytes(), 0744); err != nil {
		fmt.Println("Failed to write eqemu_config.json:", err.Error())
		os.Exit(1)
	}
}
