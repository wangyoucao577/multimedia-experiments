package util

import (
	"errors"
	"fmt"
	"io"
)

// ReadOrError read a specifed amount of data, otherwise error.
func ReadOrError(r io.Reader, data []byte) error {

	l := len(data)
	n, err := r.Read(data)
	if err != nil {
		return err
	} else if n != l {
		s := fmt.Sprintf("expect to read %d bytes but got %d bytes", l, n)
		return errors.New(s)
	}

	return nil
}
