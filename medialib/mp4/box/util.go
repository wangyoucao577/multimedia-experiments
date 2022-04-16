package box

import (
	"io"

	"github.com/golang/glog"
	"github.com/wangyoucao577/multimedia-experiments/medialib/util"
)

// ParseBox tries to parse a box from a mount of data.
// return ErrUnknownBoxType if doesn't know, otherwise fatal error.
func ParseBox(r io.Reader, pb ParentBox) (*Header, error) {
	boxHeader := Header{}
	if err := boxHeader.Parse(r); err != nil {
		if err == io.EOF {
			glog.V(1).Info("EOF")
			return nil, err
		}
		// glog.Warningf("parse box header failed, err %v", err)
		return nil, err
	}

	b, err := pb.CreateSubBox(boxHeader)
	if err != nil {
		//TODO: other types
		glog.Warningf("ignore %v %s", err, boxHeader.Type)

		// read and ignore
		//TODO: support seek
		if err := util.ReadOrError(r, make([]byte, boxHeader.PayloadSize())); err != nil {
			return &boxHeader, err
		}
		return &boxHeader, err
	}

	if err := b.ParsePayload(r); err != nil {
		glog.Warningf("parse box type %s payload failed, err %v", string(boxHeader.Type[:]), err)
		return &boxHeader, err
	}

	return &boxHeader, nil
}
