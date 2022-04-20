package main

import (
	"fmt"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
)

func printBoxes(format int) {
	var boxesStr []byte
	var err error
	switch format {
	case flagFormatYAML:
		boxesStr, err = box.BoxTypesYAML()
	case flagFormatNewLines:
		boxesStr, err = box.BoxTypesJSONIndent("", "\t")
	case flagFormatCSV:
		boxesStr, err = box.BoxesTypesCSV()
	default:
		boxesStr, err = box.BoxTypesJSON()
	}
	if err != nil {
		glog.Error(err)
	} else {
		fmt.Println(string(boxesStr))
	}
}
