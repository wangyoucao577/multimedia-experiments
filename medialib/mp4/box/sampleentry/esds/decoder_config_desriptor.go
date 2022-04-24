package esds

import (
	"encoding/binary"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// DecoderConfigDescriptor represents DecoderConfigDescriptor.
type DecoderConfigDescriptor struct {
	Tag uint8 `json:"tag"`

	ObjectTypeIndication uint8 `json:"object_type_indication"`
	StreamType           uint8 `json:"stream_type"` // 6 bits
	UpStream             uint8 `json:"up_stream"`   // 1 bit
	// 1 bit reserved here
	BufferSizeDB uint32 `json:"buffer_size_db"` // 24 bits
	MaxBitrate   uint32 `json:"max_bitrate"`
	AvgBitrate   uint32 `json:"avg_bitrate"`

	DecoderSpecificInfo DecoderSpecificInfo `json:"decoder_specific_info"`
}

func (d *DecoderConfigDescriptor) parse(r io.Reader) (uint64, error) {
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

	if err := util.ReadOrError(r, data[:1]); err != nil {
		return parsedBytes, err
	} else {
		d.ObjectTypeIndication = uint8(data[0])
		parsedBytes += 1
	}

	if err := util.ReadOrError(r, data); err != nil {
		return parsedBytes, err
	} else {
		d.StreamType = (data[0] >> 2) & 0x3F
		d.UpStream = (data[0] >> 1) & 0x1

		data[0] = 0
		d.BufferSizeDB = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	if err := util.ReadOrError(r, data); err != nil {
		return parsedBytes, err
	} else {
		d.MaxBitrate = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	if err := util.ReadOrError(r, data); err != nil {
		return parsedBytes, err
	} else {
		d.AvgBitrate = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	if bytes, err := d.DecoderSpecificInfo.parse(r); err != nil {
		return parsedBytes, err
	} else {
		parsedBytes += bytes
	}

	return parsedBytes, nil
}
