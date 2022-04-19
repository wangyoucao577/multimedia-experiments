// Package stsc represents stsc type box.
package stsc

import (
	"encoding/binary"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// ChunkEntry reprensents a chunk box entry.
type ChunkEntry struct {
	FirstChunk             uint32 `json:"first_chunk"`
	SamplesPerChunk        uint32 `json:"samples_per_chunk"`
	SampleDescriptionIndex uint32 `json:"sample_description_index"`
}

// Box represents a stsc box.
type Box struct {
	box.FullHeader `json:"full_header"`

	EntryCount uint32
	Entries    []ChunkEntry
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		FullHeader: box.FullHeader{
			Header: h,
		},
	}
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

	data := make([]byte, 4)
	if err := util.ReadOrError(r, data); err != nil {
		return err
	} else {
		b.EntryCount = binary.BigEndian.Uint32(data)
		parsedBytes += 4
	}

	for i := 0; i < int(b.EntryCount); i++ {
		entry := ChunkEntry{}

		if err := util.ReadOrError(r, data); err != nil {
			return err
		} else {
			entry.FirstChunk = binary.BigEndian.Uint32(data)
			parsedBytes += 4
		}

		if err := util.ReadOrError(r, data); err != nil {
			return err
		} else {
			entry.SamplesPerChunk = binary.BigEndian.Uint32(data)
			parsedBytes += 4
		}

		if err := util.ReadOrError(r, data); err != nil {
			return err
		} else {
			entry.SampleDescriptionIndex = binary.BigEndian.Uint32(data)
			parsedBytes += 4
		}

		b.Entries = append(b.Entries, entry)
	}

	return nil
}
