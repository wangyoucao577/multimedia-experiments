package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/exit"
	"github.com/wangyoucao577/multimedia-experiments/medialib/video/avc/es"
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
	if contentType == flagContentAVCES {
		var trackID uint32
		e := es.ElementaryStream{} // TODO: multiple stream
		for _, track := range m.Moov.Trak {
			if track.Mdia.Hdlr.HandlerType.String() == box.TypeVide {
				e.SetLengthSize(uint32(track.Mdia.Minf.Stbl.Stsd.AVC1SampleEntries[0].AVCConfig.AVCConfig.LengthSize()))
				trackID = track.Tkhd.TrackID
			}
		}

		for i := 0; i < len(m.MoofMdat); i++ {
			for _, tf := range m.MoofMdat[i].Moof.Traf {
				if tf.Tfhd.TrackID != trackID {
					continue
				}

				var startPos uint32
				for _, sampleSize := range tf.Trun[0].SampleSize {
					data := m.MoofMdat[i].Mdat.Data[startPos : startPos+sampleSize]
					e.Parse(bytes.NewReader(data), len(data))
					startPos += sampleSize
				}

				break
			}
		}

		// print AVC ES
		// var esStr []byte
		// var err error
		// switch format {
		// case flagFormatYAML:
		// 	esStr, err = .YAML()
		// case flagFormatNewLines:
		// 	esStr, err = m.Boxes.JSONIndent("", "\t")
		// case flagFormatCSV:
		// 	fallthrough // doesn't support at the moment, use default
		// default:
		// 	esStr, err = m.Boxes.JSON()
		// }

		eStr, err := json.Marshal(e)
		if err != nil {
			glog.Error(err)
		} else {
			fmt.Println(string(eStr))
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
