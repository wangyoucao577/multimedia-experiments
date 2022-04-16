// Package mvhd defines mvhd box structure.
package mvhd

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/time1904"
)

// Box represents a ftyp box.
type Box struct {
	box.FullHeader

	CreationTime     uint64
	ModificationTime uint64
	Timescale        uint32
	Duration         uint64

	Rate   int32
	Volume int16
	// reserved 16 + 2*32 = 80 bits in here
	Matrix      [9]int32
	PreDefined  [6]uint32
	NextTrackID uint32
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
	return fmt.Sprintf("FullHeader:{%v} CreationTime:%d(%s) ModificationTime:%d(%s) Timescale:%d Duration:%d(%.3fs) Rate:0x%x Volume:0x%x Matrix:%v, PreDefined:%v NextTrackID:%d",
		b.FullHeader, b.CreationTime, time1904.Unix(int64(b.CreationTime), 0).UTC(), b.ModificationTime, time1904.Unix(int64(b.ModificationTime), 0).UTC(), b.Timescale, b.Duration, float64(b.Duration)/float64(b.Timescale), b.Rate, b.Volume, b.Matrix, b.PreDefined, b.NextTrackID)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {

	// parse full header additional information first
	if err := b.FullHeader.ParseVersionFlag(r); err != nil {
		return err
	}

	// start to parse payload
	var parsedBytes uint64
	payloadSize := b.PayloadSize() // need

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
		b.Rate = int32(binary.BigEndian.Uint32(data[:4]))
		parsedBytes += 4
	}

	if err := util.ReadOrError(r, data[:2]); err != nil {
		return err
	} else {
		b.Volume = int16(binary.BigEndian.Uint16(data[:2]))
		parsedBytes += 2
	}

	// ignore reserved 16 + 2*32 = 80 bits in here
	if err := util.ReadOrError(r, make([]byte, 10)); err != nil {
		return err
	} else {
		parsedBytes += 10
	}

	data = data[:4] // shrink to 4 bytes
	for i := 0; i < len(b.Matrix); i++ {
		if err := util.ReadOrError(r, data); err != nil {
			return err
		} else {
			b.Matrix[i] = int32(binary.BigEndian.Uint32(data))
			parsedBytes += 4
		}
	}

	for i := 0; i < len(b.PreDefined); i++ {
		if err := util.ReadOrError(r, data); err != nil {
			return err
		} else {
			b.PreDefined[i] = binary.BigEndian.Uint32(data)
			parsedBytes += 4
		}
	}

	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.NextTrackID = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	if parsedBytes != payloadSize {
		return fmt.Errorf("box %s parsed bytes != payload size: %d != %d", b.Type, parsedBytes, payloadSize)
	}

	return nil
}

// CreateSubBox tries to create sub level box.
//   always return box.ErrNotImplemented since it doesn't have any sub level box.
func (b *Box) CreateSubBox(h box.Header) (box.Box, error) {
	return nil, box.ErrNotImplemented
}
