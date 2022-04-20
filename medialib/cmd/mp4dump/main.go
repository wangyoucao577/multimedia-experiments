package main

import (
	"flag"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/exit"
)

func main() {
	flag.Parse()
	defer glog.Flush()

	if flags.printBoxes {
		printBoxes(getFormatFlag())
		return
	}

	if len(flags.inputFilePath) == 0 {
		glog.Error("Input file is required.")
		exit.Fail()
	}

	parseMP4(flags.inputFilePath, getFormatFlag())

}
