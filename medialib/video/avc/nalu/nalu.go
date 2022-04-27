// Package nalu represents AVC NAL(Network Abstract Layer) Units.
package nalu

import (
	"bytes"
	"fmt"
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
	"github.com/wangyoucao577/multimedia-experiments/medialib/video/avc/nalu/aud"
	"github.com/wangyoucao577/multimedia-experiments/medialib/video/avc/nalu/sei"
)

// NALUnit represents AVC NAL Unit that defined in ISO/IEC-14496-10 7.3.1.
type NALUnit struct {
	ForbiddenZeroBit uint8 `json:"forbidden_zero_bit"` // 1 bit, shoule be 0 always
	NALRefIdc        uint8 `json:"nal_ref_idc"`        // 2 bits
	NALUnitType      uint8 `json:"nal_unit_type"`      // 5 bits
	//TODO: nal_unit_header_svc_extension

	RBRP []byte `json:"-"` // Raw byte sequence payloads

	// parsed RBRP if available
	SEIMessage          *sei.SEIMessage          `json:"sei_message,omitempty"`
	AccessUnitDelimiter *aud.AccessUnitDelimiter `json:"access_unit_delimiter,omitempty"`
}

// Parse parses bytes to AVC NAL Unit, return parsed bytes or error.
// The NAL Unit syntax defined in ISO/IEC-14496-10 7.3.1.
func (n *NALUnit) Parse(r io.Reader, size int) (uint64, error) {
	var parsedBytes uint64

	// parse nalu length
	data := make([]byte, 1)
	if err := util.ReadOrError(r, data); err != nil {
		return parsedBytes, err
	} else {
		n.ForbiddenZeroBit = (data[0] >> 7) & 0x1
		n.NALRefIdc = (data[0] >> 5) & 0x3
		n.NALUnitType = data[0] & 0x1F
		parsedBytes += 1
	}

	if n.ForbiddenZeroBit != 0 {
		return parsedBytes, fmt.Errorf("nalu forbidden_zero_bit should be 0")
	}
	if !IsValidNALUType(int(n.NALUnitType)) {
		return parsedBytes, fmt.Errorf("unknown nal_unit_type %d", n.NALUnitType)
	}

	nalUnitHeaderBytes := 1

	if n.NALUnitType == TypePrefix || n.NALUnitType == TypeSliceExtersion {
		glog.Warningf("nalu type %d svc_extension_flag and nal_unit_header_svc_extension parsing TODO", n.NALUnitType)
		//TODO: parse payload
		if err := util.ReadOrError(r, make([]byte, 2)); err != nil {
			return parsedBytes, err
		} else {
			parsedBytes += 2
			nalUnitHeaderBytes += 2
		}
	}

	next24Bits := []byte{}
	for i := nalUnitHeaderBytes; i < size; i++ {

		oneBytes := make([]byte, 1)
		if err := util.ReadOrError(r, oneBytes); err != nil {
			return parsedBytes, err
		} else {
			next24Bits = append(next24Bits, oneBytes...)
			if len(next24Bits) == 3 {
				if bytes.Equal(next24Bits, []byte{0x00, 0x00, 0x30}) {
					n.RBRP = append(n.RBRP, next24Bits[:2]...)
					next24Bits = next24Bits[3:] // ignore the emulation_prevention_three_byte 0x30
				} else {
					n.RBRP = append(n.RBRP, next24Bits[0])
					next24Bits = next24Bits[1:]
				}
			}

			parsedBytes += 1
		}

	}

	if len(next24Bits) > 0 {
		n.RBRP = append(n.RBRP, next24Bits...)
	}

	// Parse RBRP
	parser := n.prepareRBRPParser()
	if parser != nil {
		if _, err := parser.Parse(bytes.NewReader(n.RBRP), len(n.RBRP)); err != nil {
			return parsedBytes, fmt.Errorf("parse nalu type %d rbrp failed", n.NALUnitType)
		}
	} else {
		glog.Warningf("unknown nalu type %d, ignored", n.NALUnitType)
	}

	return parsedBytes, nil
}

func (n *NALUnit) prepareRBRPParser() NALUParser {
	switch n.NALUnitType {
	case TypeSEI:
		n.SEIMessage = &sei.SEIMessage{}
		return n.SEIMessage
	case TypeAccessUnitDelimiter:
		n.AccessUnitDelimiter = &aud.AccessUnitDelimiter{}
		return n.AccessUnitDelimiter
		// TODO: others
	}
	return nil
}
