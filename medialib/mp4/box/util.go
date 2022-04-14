package box

import (
	"fmt"
	"io"

	"github.com/golang/glog"
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
		if _, e := r.Read(make([]byte, boxHeader.PayloadSize())); e != nil {
			return &boxHeader, fmt.Errorf("parse box %s read %d bytes failed, err %v", boxHeader.Type, boxHeader.PayloadSize(), e)
		}
		return &boxHeader, err
	}

	if err := b.ParsePayload(r); err != nil {
		glog.Warningf("parse box type %s payload failed, err %v", string(boxHeader.Type[:]), err)
		return &boxHeader, err
	}

	return &boxHeader, nil
}
