package main

import (
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/exit"
)

func parseMP4(inputFile string, format int, contentType int) {

	// parse
	m := mp4.New(inputFile)
	if err := m.Parse(); err != nil {
		if err != io.EOF {
			glog.Errorf("Parse mp4 failed, err %v", err)
			exit.Fail()
		}
	}

	// parse avc es and print
	if contentType == flagContentESParsing {
		if es, err := m.Boxes.ExtractES(0); err != nil {
			glog.Errorf("Extract ES failed, err %v", err)
			exit.Fail()
		} else {
			// print AVC ES
			var esStr []byte
			var err error
			switch format {
			case flagFormatYAML:
				esStr, err = es.YAML()
			case flagFormatNewLines:
				esStr, err = es.JSONIndent("", "\t")
			case flagFormatCSV:
				fallthrough // doesn't support at the moment, use default
			default:
				esStr, err = es.JSON()
			}

			if err != nil {
				glog.Error(err)
			} else {
				fmt.Println(string(esStr))
			}
		}

		return
	}

	// print mp4 boxes
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
