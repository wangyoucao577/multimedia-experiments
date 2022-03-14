package main

import (
	"flag"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/gosrc/mp4"
	"github.com/wangyoucao577/multimedia-experiments/gosrc/util/exit"
)

func main() {
	flag.Parse()
	defer glog.Flush()

	if len(flags.inputFilePath) == 0 {
		glog.Error("Input file is required.")
		exit.Fail()
	}

	m := mp4.New(flags.inputFilePath)
	if err := m.Parse(); err != nil {
		if err != io.EOF {
			glog.Error("Parse mp4 failed, err %v", err)
			exit.Fail()
		}
	}

	glog.Infof("ftyp{%+v}", m.Boxes.Ftyp)

}
