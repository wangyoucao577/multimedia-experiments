package box

import "io"

// NewFunc defines generic new function to create box.
type NewFunc func(Header) Box

// Box defines interfaces for boxes.
type Box interface {

	// Parse payload. It requires BasicBox(Header) has been set to the subset Box.
	ParsePayload(r io.Reader) error

	String() string
}
