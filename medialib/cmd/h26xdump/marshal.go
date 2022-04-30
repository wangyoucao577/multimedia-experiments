package main

import "github.com/wangyoucao577/multimedia-experiments/medialib/util"

func marshalByFormat(m util.Marshaler, format int) ([]byte, error) {

	switch format {
	case flagFormatYAML:
		return m.YAML()
	case flagFormatNewLines:
		return m.JSONIndent("", "\t")
	case flagFormatCSV:
		return m.CSV()
	}

	// default
	return m.JSON()
}
