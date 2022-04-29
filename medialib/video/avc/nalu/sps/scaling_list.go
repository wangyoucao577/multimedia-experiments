package sps

import (
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util/expgolombcoding"
)

const bitsPerByte = 8

type scalingListParser struct {
	sizeOfScalingList int

	// remain bits that after parsing and no byte aligned, i.e., 1~7 bits
	remainedBits    byte
	numOfRemainBits int

	parsedBits uint64

	scalingList []int                    // result
	deltaScale  []expgolombcoding.Signed // parsed data
}

func (s *scalingListParser) parse(r io.Reader) (uint64, error) {

	lastScale := 8
	nextScale := 8

	s.scalingList = make([]int, s.sizeOfScalingList)
	for j := 0; j < s.sizeOfScalingList; j++ {

		if nextScale != 0 {
			deltaScale := expgolombcoding.Signed{}
			deltaScale.SetRemainBits(s.remainedBits, s.numOfRemainBits)
			if costBits, err := deltaScale.Parse(r); err != nil {
				return uint64(s.parsedBits), err
			} else {
				s.parsedBits += costBits
			}
			s.remainedBits, s.numOfRemainBits = deltaScale.RemainBits()
			s.deltaScale = append(s.deltaScale, deltaScale)

			nextScale = (lastScale + int(deltaScale.Value()) + 256) % 256

			//TODO: use default flags: useDefaultScalingMatrixFlag = ( j = = 0 && nextScale = = 0 )
			glog.V(3).Infof("TODO useDefaultScalingMatrixFlag")
		}

		if nextScale == 0 {
			s.scalingList[j] = lastScale
		} else {
			s.scalingList[j] = nextScale
		}
		lastScale = s.scalingList[j]
	}

	return 0, nil
}

// SetRemainBits sets bits for parsing.
//   The bits should have (8-numOfBits) padding zero bits at the beginning of the byte, which should be ignored.
//   For example, bits = [0, 0, 0, 0, 1, 0, 0, 0], numOfBit=5, then the first 3 `0` should be ignored.
//   It should be called before `Parse` if need.
func (s *scalingListParser) setRemainBits(bits byte, numOfBits int) {
	if numOfBits < 0 || numOfBits > bitsPerByte {
		glog.Warningf("invalid numOfBits %d, ignore it", numOfBits)
		return
	}

	s.remainedBits = bits
	s.numOfRemainBits = numOfBits
}

// RemainBits return remain bits and num of bits.
func (s *scalingListParser) remainBits() (byte, int) {
	return s.remainedBits, s.numOfRemainBits
}
