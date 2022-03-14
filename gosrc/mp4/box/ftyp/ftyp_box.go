// Package ftyp defines ftyp box structure.
package ftyp

import (
	"fmt"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/gosrc/mp4/box"
	"github.com/wangyoucao577/multimedia-experiments/gosrc/util"
)

// Box represents a ftyp box.
type Box struct {
	box.Box

	MajorBrand       box.FixedArray4Bytes
	MinorVersion     box.FixedArray4Bytes
	CompatibleBrands []box.FixedArray4Bytes
}

// String serializes Box.
func (b Box) String() string {
	return fmt.Sprintf("Box:{%v} MajorBrand:%s MinorVersion:%s CompatibleBrands:%v",
		b.Box, string(b.MajorBrand[:]), string(b.MinorVersion[:]), b.CompatibleBrands)
}

// ParsePayload parse payload which requires basic box already exist.
func (b *Box) ParsePayload(r io.Reader) error {

	var parsedBytes uint32

	if b.PayloadSize > 0 {
		if err := util.ReadOrError(r, b.MajorBrand[:]); err != nil {
			return err
		} else {
			parsedBytes += 4
		}

		if err := util.ReadOrError(r, b.MinorVersion[:]); err != nil {
			return err
		} else {
			parsedBytes += 4
		}

		for parsedBytes < uint32(b.PayloadSize) {
			var data [4]byte
			if err := util.ReadOrError(r, data[:]); err != nil {
				return err
			} else {
				b.CompatibleBrands = append(b.CompatibleBrands, data)
				parsedBytes += 4
			}
		}
	}

	return nil
}
