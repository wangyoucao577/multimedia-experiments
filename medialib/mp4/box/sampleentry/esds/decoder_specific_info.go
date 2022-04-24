package esds

import (
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// DecoderSpecificInfo represents DecoderSpecificInfo.
type DecoderSpecificInfo struct {
	Tag uint8 `json:"tag"`
}

func (d *DecoderSpecificInfo) parse(r io.Reader) (uint64, error) {
	var parsedBytes uint64

	data := make([]byte, 4)

	// first bytes is tag
	if err := util.ReadOrError(r, data[:1]); err != nil {
		return parsedBytes, err
	} else {
		d.Tag = data[0]
		parsedBytes += 1
	}

	// TODO: ignore 4 bytes, BUT WHY???
	if err := util.ReadOrError(r, data); err != nil {
		return parsedBytes, err
	} else {
		glog.Warningf("ignore 4 bytes data but not sure why: %v\n", data)
		parsedBytes += 4
	}

	// TODO: parse DecoderSpecificInfo
	glog.Warningln("TODO: parse DecoderSpecificInfo")

	return parsedBytes, nil
}
