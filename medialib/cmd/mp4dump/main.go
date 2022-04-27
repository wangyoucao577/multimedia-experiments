package main

import (
	"flag"
	"fmt"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/exit"
	"github.com/wangyoucao577/multimedia-experiments/medialib/video/avc/nalu"
)

func main() {
	flag.Parse()
	defer glog.Flush()

	var data []byte
	var err error

	contentFlag := getContentFlag()
	if contentFlag == flagContentBoxTypes {
		data, err = marshalByFormat(box.TypesMarshaler{}, getFormatFlag())
	} else if contentFlag == flagContentNALUTypes {
		data, err = marshalByFormat(nalu.TypesMarshaler{}, getFormatFlag())
	} else { // need to parse

		if len(flags.inputFilePath) == 0 {
			glog.Error("Input file is required.")
			exit.Fail()
		}

		data, err = parseMP4(flags.inputFilePath, getFormatFlag(), getContentFlag())

	}

	if err != nil {
		glog.Error(err)
	} else {
		fmt.Println(string(data))
	}

}
