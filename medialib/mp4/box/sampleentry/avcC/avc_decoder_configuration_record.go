package avcc

import (
	"encoding/binary"
	"io"

	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// AVCDecoderConfigurationRecord defines AVC Decoder configuration record.
type AVCDecoderConfigurationRecord struct {
	ConfigurationVersion uint8 `json:"configuration_version"`
	AVCProfileIndication uint8 `json:"avc_profile_indication"`
	ProfileCompatibility uint8 `json:"profile_compatibility"`
	AVCLevelIndication   uint8 `json:"avc_level_indication"`

	// 6 bits reserved here
	LengthSizeMinusOne uint8 `json:"length_size_minus_one"` // 2 bits in file

	// sps
	// 3 bits reserved here
	NumOfSequenceParameterSets  uint8    `json:"num_of_sequence_parameter_sets"` // 5 bits in file
	SequenceParameterSetLength  []uint16 `json:"sequence_parameter_set_length"`
	SequenceParameterSetNALUnit [][]byte `json:"sequence_parameter_set_nal_unit"`

	// pps
	NumOfPictureParameterSets  uint8    `json:"num_of_picture_parameter_sets"`
	PictureParameterSetLength  []uint16 `json:"picture_parameter_set_length"`
	PictureParameterSetNALUnit [][]byte `json:"picture_parameter_set_nal_unit"`

	// 6 bits reserved here
	ChromaFormat uint8 `json:"chroma_format"` // 2 bits in file
	// 5 bits reserved here
	BitDepthLumaMinus8 uint8 `json:"bit_depth_luma_minus8"` // 3 bits in file
	// 5 bits reserved here
	BitDepthChromaMinus8 uint8 `json:"bit_depth_chroma_minus8"` // 3 bits in file

	// sps extensions
	NumOfSequenceParameterSetExt   uint8    `json:"num_of_sequence_parameter_set_ext"`
	SequenceParameterSetExtLength  []uint16 `json:"sequence_parameter_set_ext_length"`
	SequenceParameterSetExtNALUnit [][]byte `json:"sequence_parameter_set_ext_nal_unit"`
}

// LengthSize returns NALU prefix length size.
func (a *AVCDecoderConfigurationRecord) LengthSize() uint32 {
	return uint32(a.LengthSizeMinusOne) + 1
}

func (a *AVCDecoderConfigurationRecord) parse(r io.Reader) (uint64, error) {

	var parsedBytes uint64

	data := make([]byte, 4)

	if err := util.ReadOrError(r, data); err != nil {
		return parsedBytes, err
	} else {
		a.ConfigurationVersion = data[0]
		a.AVCProfileIndication = data[1]
		a.ProfileCompatibility = data[2]
		a.AVCLevelIndication = data[3]

		parsedBytes += 4
	}

	if err := util.ReadOrError(r, data[:2]); err != nil {
		return parsedBytes, err
	} else {
		a.LengthSizeMinusOne = data[0] & 0x3
		a.NumOfSequenceParameterSets = data[1] & 0x1F
		parsedBytes += 2
	}

	for i := 0; i < int(a.NumOfSequenceParameterSets); i++ {
		var len uint16
		if err := util.ReadOrError(r, data[:2]); err != nil {
			return parsedBytes, err
		} else {
			len = binary.BigEndian.Uint16(data[:2])
			a.SequenceParameterSetLength = append(a.SequenceParameterSetLength, len)
			parsedBytes += 2
		}

		nal := make([]byte, len)
		if err := util.ReadOrError(r, nal); err != nil {
			return parsedBytes, err
		} else {
			a.SequenceParameterSetNALUnit = append(a.SequenceParameterSetNALUnit, nal)
			parsedBytes += uint64(len)
		}
	}

	if err := util.ReadOrError(r, data[:1]); err != nil {
		return parsedBytes, err
	} else {
		a.NumOfPictureParameterSets = data[0]
		parsedBytes += 1
	}

	for i := 0; i < int(a.NumOfPictureParameterSets); i++ {
		var len uint16
		if err := util.ReadOrError(r, data[:2]); err != nil {
			return parsedBytes, err
		} else {
			len = binary.BigEndian.Uint16(data[:2])
			a.PictureParameterSetLength = append(a.PictureParameterSetLength, len)
			parsedBytes += 2
		}

		nal := make([]byte, len)
		if err := util.ReadOrError(r, nal); err != nil {
			return parsedBytes, err
		} else {
			a.PictureParameterSetNALUnit = append(a.PictureParameterSetNALUnit, nal)
			parsedBytes += uint64(len)
		}
	}

	// TODO: Need to check profile_idc
	// if err := util.ReadOrError(r, data); err != nil {
	// 	return parsedBytes, err
	// } else {
	// 	a.ChromaFormat = data[0] & 0x3
	// 	a.BitDepthLumaMinus8 = data[1] & 0x7
	// 	a.BitDepthChromaMinus8 = data[2] & 0x7
	// 	a.NumOfSequenceParameterSetExt = data[3]

	// 	parsedBytes += 4
	// }

	// for i := 0; i < int(a.NumOfSequenceParameterSetExt); i++ {
	// 	var len uint16
	// 	if err := util.ReadOrError(r, data[:2]); err != nil {
	// 		return parsedBytes, err
	// 	} else {
	// 		len = binary.BigEndian.Uint16(data[:2])
	// 		a.SequenceParameterSetExtLength = append(a.SequenceParameterSetExtLength, len)
	// 		parsedBytes += 2
	// 	}

	// 	nal := make([]byte, len)
	// 	if err := util.ReadOrError(r, nal); err != nil {
	// 		return parsedBytes, err
	// 	} else {
	// 		a.SequenceParameterSetExtNALUnit = append(a.SequenceParameterSetExtNALUnit, nal)
	// 		parsedBytes += uint64(len)
	// 	}
	// }

	return parsedBytes, nil
}
