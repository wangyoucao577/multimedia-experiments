// Package sps defined AVC Sequence Parameter Sets information.
package sps

import (
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/expgolombcoding"
)

// SequenceParameterSetData represents SequenceParameterSetData defined in ISO/IEC-14496-10 7.3.2.
type SequenceParameterSetData struct {
	ProfileIdc         uint8 `json:"profile_idc"`
	ConstraintSet0Flag uint8 `json:"constraint_set0_flag"` // 1 bit
	ConstraintSet1Flag uint8 `json:"constraint_set1_flag"` // 1 bit
	ConstraintSet2Flag uint8 `json:"constraint_set2_flag"` // 1 bit
	ConstraintSet3Flag uint8 `json:"constraint_set3_flag"` // 1 bit
	ConstraintSet4Flag uint8 `json:"constraint_set4_flag"` // 1 bit
	ConstraintSet5Flag uint8 `json:"constraint_set5_flag"` // 1 bit
	// 2 bytes reserved here
	LevelIdc                        uint8                      `json:"level_idc"`
	SeqParameterSetID               expgolombcoding.Unsigned   `json:"seq_parameter_set_id"`                           // Exp-Golomb-coded
	ChromaFormatIdc                 *expgolombcoding.Unsigned  `json:"chroma_format_idc,omitempty"`                    // Exp-Golomb-coded
	SeparateColourPlaneFlag         *uint8                     `json:"separate_colour_plane_flag,omitempty"`           // 1 bit
	BitDepthLumaMinus8              *expgolombcoding.Unsigned  `json:"bit_depth_luma_minus8,omitempty"`                // Exp-Golomb-coded
	BitDepthChromaMinus8            *expgolombcoding.Unsigned  `json:"bit_depth_chroma_minus8,omitempty"`              // Exp-Golomb-coded
	QpprimeYZeroTransformBypassFlag *uint8                     `json:"qpprime_y_zero_transform_bypass_flag,omitempty"` // 1 bit
	SeqScalingMatrixPresentFlag     *uint8                     `json:"seq_scaling_matrix_present_flag,omitempty"`      // 1 bit
	SeqScalingListPresentFlag       []uint8                    `json:"seq_scaling_list_present_flag,omitempty"`        // 1 bit per flag
	DeltaScale                      [][]expgolombcoding.Signed `json:"delta_scale,omitempty"`
	ScalingList4x4                  [][]int                    `json:"scaling_list_4x4,omitempty"`
	ScalingList8x8                  [][]int                    `json:"scaling_list_8x8,omitempty"`
	Log2MaxFrameNumMinus4           expgolombcoding.Unsigned   `json:"log2_max_frame_num_minus4"`                       // Exp-Golomb-coded
	PicOrderCntType                 expgolombcoding.Unsigned   `json:"pic_order_cnt_type"`                              // Exp-Golomb-coded
	Log2MaxPicOrderCntLsbMinus4     *expgolombcoding.Unsigned  `json:"log2_max_pic_order_cnt_lsb_minus4,omitempty"`     // Exp-Golomb-coded
	DeltaPicOrderAlwaysZeroFlag     *uint8                     `json:"delta_pic_order_always_zero_flag,omitempty"`      // 1 bit
	OffsetForNonRefPic              *expgolombcoding.Signed    `json:"offset_for_non_ref_pic,omitempty"`                // Exp-Golomb-coded
	OffsetForTopToBottomField       *expgolombcoding.Signed    `json:"offset_for_top_to_bottom_field,omitempty"`        // Exp-Golomb-coded
	NumRefFramesInPicOrderCntCycle  *expgolombcoding.Unsigned  `json:"num_ref_frames_in_pic_order_cnt_cycle,omitempty"` // Exp-Golomb-coded
	OffsetForRefFrame               []expgolombcoding.Signed   `json:"offset_for_ref_frame,omitempty"`                  // Exp-Golomb-coded
	MaxNumRefFrames                 expgolombcoding.Unsigned   `json:"max_num_ref_frames"`                              // Exp-Golomb-coded
	GapsInFrameNumValueAllowedFlag  uint8                      `json:"gaps_in_frame_num_value_allowed_flag"`            // 1 bit
	PicWidthInMbsMinus1             expgolombcoding.Unsigned   `json:"pic_width_in_mbs_minus1"`                         // Exp-Golomb-coded
	PicHeightInMapUnitsMinus1       expgolombcoding.Unsigned   `json:"pic_height_in_map_units_minus1"`                  // Exp-Golomb-coded
	FrameMbsOnlyFlag                uint8                      `json:"frame_mbs_only_flag"`                             // 1 bit
	MbAdaptiveFrameFieldFlag        *uint8                     `json:"mb_adaptive_frame_field_flag,omitempty"`          // 1 bit
	Direct8x8InferenceFlag          uint8                      `json:"direct_8x8_inference_flag"`                       // 1 bit
	FrameCroppingFlag               uint8                      `json:"frame_cropping_flag"`                             // 1 bit
	FrameCropLeftOffset             *expgolombcoding.Unsigned  `json:"frame_crop_left_offset,omitempty"`                // Exp-Golomb-coded
	FrameCropRightOffset            *expgolombcoding.Unsigned  `json:"frame_crop_right_offset,omitempty"`               // Exp-Golomb-coded
	FrameCropTopOffset              *expgolombcoding.Unsigned  `json:"frame_crop_top_offset,omitempty"`                 // Exp-Golomb-coded
	FrameCropBottomOffset           *expgolombcoding.Unsigned  `json:"frame_crop_bottom_offset,omitempty"`              // Exp-Golomb-coded
	VuiParametersPresentFlag        uint8                      `json:"vui_parameters_present_flag"`                     // 1 bit
	//TODO: vui_parameters()
}

// Parse parses bytes to AVC SPS NAL Unit, return parsed bytes or error.
func (s *SequenceParameterSetData) Parse(r io.Reader, size int) (uint64, error) {
	var parsedBytes uint64

	nextByte := make([]byte, 1)

	if err := util.ReadOrError(r, nextByte); err != nil {
		return parsedBytes, err
	} else {
		s.ProfileIdc = nextByte[0]
		parsedBytes += 1
	}

	if err := util.ReadOrError(r, nextByte); err != nil {
		return parsedBytes, err
	} else {
		s.ConstraintSet0Flag = (nextByte[0] >> 7) & 0x1
		s.ConstraintSet1Flag = (nextByte[0] >> 6) & 0x1
		s.ConstraintSet2Flag = (nextByte[0] >> 5) & 0x1
		s.ConstraintSet3Flag = (nextByte[0] >> 4) & 0x1
		s.ConstraintSet4Flag = (nextByte[0] >> 3) & 0x1
		s.ConstraintSet5Flag = (nextByte[0] >> 2) & 0x1

		parsedBytes += 1
	}

	if err := util.ReadOrError(r, nextByte); err != nil {
		return parsedBytes, err
	} else {
		s.LevelIdc = nextByte[0]
		parsedBytes += 1
	}

	var nextCostBits uint64

	if costBits, err := s.SeqParameterSetID.Parse(r); err != nil {
		return parsedBytes, err
	} else {
		nextCostBits += costBits
	}
	remainBits, numOfRemainBits := s.SeqParameterSetID.RemainBits()

	// ISO/IEC-14496-10 7.3.2.1.1
	if s.ProfileIdc == 100 || s.ProfileIdc == 110 || s.ProfileIdc == 122 ||
		s.ProfileIdc == 244 || s.ProfileIdc == 44 || s.ProfileIdc == 83 ||
		s.ProfileIdc == 86 || s.ProfileIdc == 118 || s.ProfileIdc == 128 {

		expUnsigned := &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.ChromaFormatIdc = expUnsigned

		if s.ChromaFormatIdc.Value() == 3 {
			if numOfRemainBits > 0 {
				v := (remainBits >> (numOfRemainBits - 1)) & 0x1
				s.SeparateColourPlaneFlag = &v
				numOfRemainBits--
			} else {
				if err := util.ReadOrError(r, nextByte); err != nil {
					return parsedBytes, err
				} else {
					v := (nextByte[0] >> 7) & 0x1
					s.SeparateColourPlaneFlag = &v
					remainBits = nextByte[0]
					numOfRemainBits = 7 // 8-1
				}
			}
			nextCostBits += 1
		}

		expUnsigned = &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
			glog.Warningf("BitDepthLumaMinus8 cost bits %d", 1)
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.BitDepthLumaMinus8 = expUnsigned

		expUnsigned = &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
			glog.Warningf("BitDepthChromaMinus8 cost bits %d", 1)
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.BitDepthChromaMinus8 = expUnsigned

		if numOfRemainBits > 0 {
			v := (remainBits >> (numOfRemainBits - 1)) & 0x1
			s.QpprimeYZeroTransformBypassFlag = &v
			numOfRemainBits--
		} else {
			if err := util.ReadOrError(r, nextByte); err != nil {
				return parsedBytes, err
			} else {
				v := (nextByte[0] >> 7) & 0x1
				s.QpprimeYZeroTransformBypassFlag = &v
				remainBits = nextByte[0]
				numOfRemainBits = 7 // 8-1
			}
		}
		nextCostBits += 1

		if numOfRemainBits > 0 {
			v := (remainBits >> (numOfRemainBits - 1)) & 0x1
			s.SeqScalingMatrixPresentFlag = &v
			numOfRemainBits--
		} else {
			if err := util.ReadOrError(r, nextByte); err != nil {
				return parsedBytes, err
			} else {
				v := (nextByte[0] >> 7) & 0x1
				s.SeqScalingMatrixPresentFlag = &v
				remainBits = nextByte[0]
				numOfRemainBits = 7 // 8-1
			}
		}
		nextCostBits += 1

		if s.SeqScalingMatrixPresentFlag != nil && *s.SeqScalingMatrixPresentFlag != 0 {

			scalingListPresentFlagLen := 12
			if s.ChromaFormatIdc.Value() != 3 {
				scalingListPresentFlagLen = 8
			}

			for i := 0; i < scalingListPresentFlagLen; i++ {
				if numOfRemainBits > 0 {
					presentFlag := (remainBits >> (numOfRemainBits - 1)) & 0x1
					s.SeqScalingListPresentFlag = append(s.SeqScalingListPresentFlag, presentFlag)
					numOfRemainBits--
				} else {
					if err := util.ReadOrError(r, nextByte); err != nil {
						return parsedBytes, err
					} else {
						presentFlag := (nextByte[0] >> 7) & 0x1
						s.SeqScalingListPresentFlag = append(s.SeqScalingListPresentFlag, presentFlag)
						remainBits = nextByte[0]
						numOfRemainBits = 7 // 8-1
					}
				}
				nextCostBits += 1

				if s.SeqScalingListPresentFlag[i] != 0 {
					if i < 6 {
						sp := scalingListParser{sizeOfScalingList: 16}
						sp.setRemainBits(remainBits, numOfRemainBits)
						if costBits, err := sp.parse(r); err != nil {
							return parsedBytes, err
						} else {
							nextCostBits += costBits
						}
						remainBits, numOfRemainBits = sp.remainBits()
						s.ScalingList4x4 = append(s.ScalingList4x4, sp.scalingList)
						s.DeltaScale = append(s.DeltaScale, sp.deltaScale)
					} else {
						sp := scalingListParser{sizeOfScalingList: 64}
						sp.setRemainBits(remainBits, numOfRemainBits)
						if costBits, err := sp.parse(r); err != nil {
							return parsedBytes, err
						} else {
							nextCostBits += costBits
						}
						remainBits, numOfRemainBits = sp.remainBits()
						s.ScalingList8x8 = append(s.ScalingList8x8, sp.scalingList)
						s.DeltaScale = append(s.DeltaScale, sp.deltaScale)
					}
				}
				glog.Warningf("Scaling List parsing maybe wrong, check the results first!!!")
			}
		}
	}

	s.Log2MaxFrameNumMinus4.SetRemainBits(remainBits, numOfRemainBits)
	if costBits, err := s.Log2MaxFrameNumMinus4.Parse(r); err != nil {
		return parsedBytes, err
	} else {
		nextCostBits += costBits
	}
	remainBits, numOfRemainBits = s.Log2MaxFrameNumMinus4.RemainBits()

	s.PicOrderCntType.SetRemainBits(remainBits, numOfRemainBits)
	if costBits, err := s.PicOrderCntType.Parse(r); err != nil {
		return parsedBytes, err
	} else {
		nextCostBits += costBits
	}
	remainBits, numOfRemainBits = s.PicOrderCntType.RemainBits()

	if s.PicOrderCntType.Value() == 0 {
		expUnsigned := &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.Log2MaxPicOrderCntLsbMinus4 = expUnsigned
	} else if s.PicOrderCntType.Value() == 1 {
		if numOfRemainBits > 0 {
			v := (remainBits >> (numOfRemainBits - 1)) & 0x1
			s.DeltaPicOrderAlwaysZeroFlag = &v
			numOfRemainBits--
		} else {
			if err := util.ReadOrError(r, nextByte); err != nil {
				return parsedBytes, err
			} else {
				v := (nextByte[0] >> 7) & 0x1
				s.DeltaPicOrderAlwaysZeroFlag = &v
				remainBits = nextByte[0]
				numOfRemainBits = 7 // 8-1
			}
		}
		nextCostBits += 1

		expSigned := &expgolombcoding.Signed{}
		expSigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expSigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expSigned.RemainBits()
		s.OffsetForNonRefPic = expSigned

		expSigned = &expgolombcoding.Signed{}
		expSigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expSigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expSigned.RemainBits()
		s.OffsetForTopToBottomField = expSigned

		expUnsigned := &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.NumRefFramesInPicOrderCntCycle = expUnsigned

		for i := 0; i < int(s.NumRefFramesInPicOrderCntCycle.Value()); i++ {
			expSigned := expgolombcoding.Signed{}
			expSigned.SetRemainBits(remainBits, numOfRemainBits)
			if costBits, err := expSigned.Parse(r); err != nil {
				return parsedBytes, err
			} else {
				s.OffsetForRefFrame = append(s.OffsetForRefFrame, expSigned)
				nextCostBits += costBits
			}
			remainBits, numOfRemainBits = expSigned.RemainBits()
		}
	}

	s.MaxNumRefFrames.SetRemainBits(remainBits, numOfRemainBits)
	if costBits, err := s.MaxNumRefFrames.Parse(r); err != nil {
		return parsedBytes, err
	} else {
		nextCostBits += costBits
	}
	remainBits, numOfRemainBits = s.MaxNumRefFrames.RemainBits()

	if numOfRemainBits > 0 {
		s.GapsInFrameNumValueAllowedFlag = (remainBits >> (numOfRemainBits - 1)) & 0x1
		numOfRemainBits--
	} else {
		if err := util.ReadOrError(r, nextByte); err != nil {
			return parsedBytes, err
		} else {
			s.GapsInFrameNumValueAllowedFlag = (nextByte[0] >> 7) & 0x1
			remainBits = nextByte[0]
			numOfRemainBits = 7 // 8-1
		}
	}
	nextCostBits += 1

	s.PicWidthInMbsMinus1.SetRemainBits(remainBits, numOfRemainBits)
	if costBits, err := s.PicWidthInMbsMinus1.Parse(r); err != nil {
		return parsedBytes, err
	} else {
		nextCostBits += costBits
	}
	remainBits, numOfRemainBits = s.PicWidthInMbsMinus1.RemainBits()

	s.PicHeightInMapUnitsMinus1.SetRemainBits(remainBits, numOfRemainBits)
	if costBits, err := s.PicHeightInMapUnitsMinus1.Parse(r); err != nil {
		return parsedBytes, err
	} else {
		nextCostBits += costBits
	}
	remainBits, numOfRemainBits = s.PicHeightInMapUnitsMinus1.RemainBits()

	if numOfRemainBits > 0 {
		s.FrameMbsOnlyFlag = (remainBits >> (numOfRemainBits - 1)) & 0x1
		numOfRemainBits--
	} else {
		if err := util.ReadOrError(r, nextByte); err != nil {
			return parsedBytes, err
		} else {
			s.FrameMbsOnlyFlag = (nextByte[0] >> 7) & 0x1
			remainBits = nextByte[0]
			numOfRemainBits = 7 // 8-1
		}
	}
	nextCostBits += 1

	if s.FrameMbsOnlyFlag == 0 {
		if numOfRemainBits > 0 {
			v := (remainBits >> (numOfRemainBits - 1)) & 0x1
			s.MbAdaptiveFrameFieldFlag = &v
			numOfRemainBits--
		} else {
			if err := util.ReadOrError(r, nextByte); err != nil {
				return parsedBytes, err
			} else {
				v := (nextByte[0] >> 7) & 0x1
				s.MbAdaptiveFrameFieldFlag = &v
				remainBits = nextByte[0]
				numOfRemainBits = 7 // 8-1
			}
		}
		nextCostBits += 1
	}

	if numOfRemainBits > 0 {
		s.Direct8x8InferenceFlag = (remainBits >> (numOfRemainBits - 1)) & 0x1
		numOfRemainBits--
	} else {
		if err := util.ReadOrError(r, nextByte); err != nil {
			return parsedBytes, err
		} else {
			s.Direct8x8InferenceFlag = (nextByte[0] >> 7) & 0x1
			remainBits = nextByte[0]
			numOfRemainBits = 7 // 8-1
		}
	}
	nextCostBits += 1

	if numOfRemainBits > 0 {
		s.FrameCroppingFlag = (remainBits >> (numOfRemainBits - 1)) & 0x1
		numOfRemainBits--
	} else {
		if err := util.ReadOrError(r, nextByte); err != nil {
			return parsedBytes, err
		} else {
			s.FrameCroppingFlag = (nextByte[0] >> 7) & 0x1
			remainBits = nextByte[0]
			numOfRemainBits = 7 // 8-1
		}
	}
	nextCostBits += 1

	if s.FrameCroppingFlag != 0 {
		expUnsigned := &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.FrameCropLeftOffset = expUnsigned

		expUnsigned = &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.FrameCropRightOffset = expUnsigned

		expUnsigned = &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.FrameCropTopOffset = expUnsigned

		expUnsigned = &expgolombcoding.Unsigned{}
		expUnsigned.SetRemainBits(remainBits, numOfRemainBits)
		if costBits, err := expUnsigned.Parse(r); err != nil {
			return parsedBytes, err
		} else {
			nextCostBits += costBits
		}
		remainBits, numOfRemainBits = expUnsigned.RemainBits()
		s.FrameCropBottomOffset = expUnsigned
	}

	if numOfRemainBits > 0 {
		s.VuiParametersPresentFlag = (remainBits >> (numOfRemainBits - 1)) & 0x1
		numOfRemainBits--
	} else {
		if err := util.ReadOrError(r, nextByte); err != nil {
			return parsedBytes, err
		} else {
			s.VuiParametersPresentFlag = (nextByte[0] >> 7) & 0x1
			remainBits = nextByte[0]
			numOfRemainBits = 7 // 8-1
		}
	}
	nextCostBits += 1

	if s.VuiParametersPresentFlag != 0 {
		//TODO:
		glog.Warningf("vui_parameters need to parse")
		parsedBytes += nextCostBits / 8
		if nextCostBits%8 != 0 {
			parsedBytes += 1
		}
		return parsedBytes, nil
	}

	parsedBytes += nextCostBits / 8
	if nextCostBits%8 != 0 {
		glog.Warningf("bits doesn't align in 8 bits, remain %d bits", nextCostBits%8)
		parsedBytes += 1
	}
	if int(parsedBytes) != size {
		glog.Warningf("parsed bytes != expect size : %d!=%d", parsedBytes, size)
	}

	return parsedBytes, nil
}
