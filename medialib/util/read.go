package util

import (
	"errors"
	"fmt"
	"io"
)

// ReadOrError read a specifed amount of data, otherwise error.
func ReadOrError(r io.Reader, data []byte) error {

	n, err := r.Read(data)
	if err != nil {
		return err
	} else if n != len(data) {
		s := fmt.Sprintf("expect to read %d bytes but got %d bytes", len(data), n)
		return errors.New(s)
	}

	return nil
}
