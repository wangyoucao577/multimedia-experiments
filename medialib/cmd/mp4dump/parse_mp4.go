package main

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/exit"
)

func parseMP4(inputFile string, format int) {

	// parse
	m := mp4.New(inputFile)
	if err := m.Parse(); err != nil {
		if err != io.EOF {
			glog.Errorf("Parse mp4 failed, err %v", err)
			exit.Fail()
		}
	}

	// print
	var boxesStr []byte
	var err error
	switch format {
	case flagFormatYAML:
		boxesStr, err = m.Boxes.YAML()
	case flagFormatNewLines:
		boxesStr, err = m.Boxes.JSONIndent("", "\t")
	case flagFormatCSV:
		fallthrough // doesn't support at the moment, use default
	default:
		boxesStr, err = m.Boxes.JSON()
	}
	if err != nil {
		glog.Error(err)
	} else {
		fmt.Println(string(boxesStr))
	}
}
