package main

import (
	"flag"
	"strings"
)

var flags struct {
	inputFilePath string
	format        string
}

func init() {
	flag.StringVar(&flags.inputFilePath, "i", "", "Input mp4 file path.")
	flag.StringVar(&flags.format, "format", "json", "Output format, available values:json,json_newlines,yaml")
}

const (
	flagFormatJSON = iota
	flagFormatNewLines
	flagFormatYAML
)

func getFormatFlag() int {
	switch strings.ToLower(flags.format) {
	case "yaml":
		return flagFormatYAML
	case "json_newlines":
		return flagFormatNewLines
	}
	return flagFormatJSON
}
