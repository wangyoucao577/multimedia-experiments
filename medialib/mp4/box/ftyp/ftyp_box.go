// Package ftyp defines ftyp box structure.
package ftyp

import (
	"encoding/binary"
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// Box represents a ftyp box.
type Box struct {
	box.Header

	MajorBrand       box.FixedArray4Bytes
	MinorVersion     uint32
	CompatibleBrands []box.FixedArray4Bytes
}

// New creates a new Box.
func New(h box.Header) box.Box {
	return &Box{
		Header: h,
	}
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Header:{%v} MajorBrand:%s MinorVersion:%d CompatibleBrands:%v",
		b.Header, string(b.MajorBrand[:]), b.MinorVersion, b.CompatibleBrands)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {

	var parsedBytes uint32
	payloadSize := b.PayloadSize()

	if err := util.ReadOrError(r, b.MajorBrand[:]); err != nil {
		return err
	} else {
		parsedBytes += 4
	}

	minorVersionData := make([]byte, 4)
	if err := util.ReadOrError(r, minorVersionData); err != nil {
		return err
	} else {
		b.MinorVersion = binary.BigEndian.Uint32(minorVersionData)
		parsedBytes += 4
	}

	for parsedBytes < uint32(payloadSize) {
		var data [4]byte
		if err := util.ReadOrError(r, data[:]); err != nil {
			return err
		} else {
			b.CompatibleBrands = append(b.CompatibleBrands, data)
			parsedBytes += 4
		}
	}

	if parsedBytes != uint32(payloadSize) {
		return fmt.Errorf("box %s parsed bytes != payload size: %d != %d", b.Type, parsedBytes, payloadSize)
	}

	return nil
}
