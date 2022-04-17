// Package mdhd represents mdhd type box.
package mdhd

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/time1904"
)

// Box represents a mdhd box.
type Box struct {
	box.FullHeader

	CreationTime     uint64
	ModificationTime uint64
	Timescale        uint32
	Duration         uint64

	Pad        uint8    // 1 bit
	Language   [3]uint8 // 5 bytes per uint
	Predefined uint16
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		FullHeader: box.FullHeader{
			Header: h,
		},
	}
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("FullHeader:{%v} CreationTime:%d(%s) ModificationTime:%d(%s) Timescale:%d Duration:%d(%.3fs) Pad:%d Language:%v, PreDefined:%d",
		b.FullHeader, b.CreationTime, time1904.Unix(int64(b.CreationTime), 0).UTC(), b.ModificationTime, time1904.Unix(int64(b.ModificationTime), 0).UTC(), b.Timescale, b.Duration, float64(b.Duration)/float64(b.Timescale), b.Pad, b.Language, b.Predefined)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {
	if b.PayloadSize() == 0 {
		glog.Warningf("box %s is empty", b.Type)
		return nil
	}

	// parse full header additional information first
	if err := b.FullHeader.ParseVersionFlag(r); err != nil {
		return err
	}

	// start to parse payload
	var parsedBytes uint64

	timeDataSize := 4 // if Version == 0
	if b.FullHeader.Version == 1 {
		timeDataSize = 8
	}
	data := make([]byte, timeDataSize)

	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		if timeDataSize == 8 {
			b.CreationTime = binary.BigEndian.Uint64(data)
		} else {
			b.CreationTime = uint64(binary.BigEndian.Uint32(data))
		}
		parsedBytes += uint64(timeDataSize)
	}

	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		if timeDataSize == 8 {
			b.ModificationTime = binary.BigEndian.Uint64(data)
		} else {
			b.ModificationTime = uint64(binary.BigEndian.Uint32(data))
		}
		parsedBytes += uint64(timeDataSize)
	}

	if err := util.ReadOrError(r, data[:4]); err != nil {
		return err
	} else {
		b.Timescale = binary.BigEndian.Uint32(data[:4])
		parsedBytes += 4
	}

	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		if timeDataSize == 8 {
			b.Duration = binary.BigEndian.Uint64(data)
		} else {
			b.Duration = uint64(binary.BigEndian.Uint32(data))
		}
		parsedBytes += uint64(timeDataSize)
	}

	if err := util.ReadOrError(r, data[:4]); err != nil {
		return err
	} else {
		b.Pad = (uint8(data[0]) >> 7) & 0x1 // 1 bit

		// 5 bits per Language
		b.Language[0] = (uint8(data[0]) >> 2) & 0x1F
		b.Language[1] = ((uint8(data[0]) & 0x3) << 3) | ((uint8(data[1]) >> 5) & 0x7)
		b.Language[2] = uint8(data[1]) & 0x1F

		b.Predefined = binary.BigEndian.Uint16(data[2:4])

		parsedBytes += 4
	}

	if parsedBytes != b.PayloadSize() {
		return fmt.Errorf("box %s parsed bytes != payload size: %d != %d", b.Type, parsedBytes, b.PayloadSize())
	}

	return nil
}
