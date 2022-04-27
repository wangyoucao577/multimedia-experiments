package util

// Marshaler defines serveral Marshal interfaces in one place.
type Marshaler interface {

	// JSON marshals object to JSON represtation.
	JSON() ([]byte, error)

	// JSONIndent marshals object to JSON with indent represent, it has same parameters with json.MarshalIndent.
	JSONIndent(prefix, indent string) ([]byte, error)

	// YAML marshals object to YAML representation.
	YAML() ([]byte, error)

	// CSV marshals object to CSV representation.
	CSV() ([]byte, error)
}
