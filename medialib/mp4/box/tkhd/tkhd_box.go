// Package tkhd represents tkhd type box.
package tkhd

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/time1904"
)

// Box represents a tkhd box.
type Box struct {
	box.FullHeader

	CreationTime     uint64
	ModificationTime uint64
	// reserved 4 byes here
	TrackID  uint32
	Duration uint64
	// reserved 4*2 bytes here
	Layer            int16
	AlternativeGroup int16
	Volume           int16
	// reserved 2 bytes here
	Matrix [9]int32
	Width  uint32
	Height uint32
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
	return fmt.Sprintf("FullHeader:{%v} CreationTime:%d(%s) ModificationTime:%d(%s) TrackID:%d Duration:%d Layer:0x%x AlternativeGroup 0x%x Volume:0x%x Matrix:%v, Width:%v Height:%d",
		b.FullHeader, b.CreationTime, time1904.Unix(int64(b.CreationTime), 0).UTC(), b.ModificationTime, time1904.Unix(int64(b.ModificationTime), 0).UTC(), b.TrackID, b.Duration, b.Layer, b.AlternativeGroup, b.Volume, b.Matrix, b.Width, b.Height)
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
		b.TrackID = binary.BigEndian.Uint32(data[:4])
		parsedBytes += 4
	}

	// ignore reserved 4 bytes in here
	if err := util.ReadOrError(r, data[:4]); err != nil {
		return err
	} else {
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

	// ignore reserved 4*2 bytes in here
	if err := util.ReadOrError(r, make([]byte, 8)); err != nil {
		return err
	} else {
		parsedBytes += 8
	}

	if err := util.ReadOrError(r, data[:2]); err != nil {
		return err
	} else {
		b.Layer = int16(binary.BigEndian.Uint16(data[:2]))
		parsedBytes += 2
	}

	if err := util.ReadOrError(r, data[:2]); err != nil {
		return err
	} else {
		b.AlternativeGroup = int16(binary.BigEndian.Uint16(data[:2]))
		parsedBytes += 2
	}

	if err := util.ReadOrError(r, data[:2]); err != nil {
		return err
	} else {
		b.Volume = int16(binary.BigEndian.Uint16(data[:2]))
		parsedBytes += 2
	}

	// ignore reserved 2 bytes in here
	if err := util.ReadOrError(r, data[:2]); err != nil {
		return err
	} else {
		parsedBytes += 2
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

	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.Width = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.Height = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	if parsedBytes != payloadSize {
		return fmt.Errorf("box %s parsed bytes != payload size: %d != %d", b.Type, parsedBytes, payloadSize)
	}

	return nil
}
