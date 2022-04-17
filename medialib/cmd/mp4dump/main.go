package main

import (
	"flag"
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/exit"
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
			glog.Errorf("Parse mp4 failed, err %v", err)
			exit.Fail()
		}
	}

	if d, err := m.Boxes.MarshalString(false); err != nil {
		glog.Error(err)
	} else {
		fmt.Println(d)
	}
}
