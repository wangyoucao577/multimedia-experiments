package main

import (
	"io"
	"os"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/exit"
)

func parseMP4(inputFile string, format int, contentType int) ([]byte, error) {

	// parse
	m := mp4.New(inputFile)
	if err := m.Parse(); err != nil {
		if err != io.EOF {
			glog.Errorf("Parse mp4 failed, err %v", err)
			exit.Fail()
		}
	}

	// parse avc/hevc es and print
	if contentType == flagContentES || contentType == flagContentRawES {
		if es, err := m.Boxes.ExtractES(0); err != nil {
			glog.Errorf("Extract ES failed, err %v", err)
			exit.Fail()
		} else {
			// print AVC ES
			if contentType == flagContentRawES {
				if _, err := es.Dump(os.Stdout); err != nil {
					glog.Errorf("Dump ES failed, err %v", err)
					exit.Fail()
				}
				return nil, nil
			} else {
				return marshalByFormat(es, format)
			}
		}
	}

	// print mp4 boxes
	return marshalByFormat(m.Boxes, format)
}
