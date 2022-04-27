package main

import (
	"flag"
	"strings"
)

var flags struct {
	inputFilePath string
	content       string // content to output
	format        string
}

func init() {
	flag.StringVar(&flags.inputFilePath, "i", "", "Input mp4 file path.")
	flag.StringVar(&flags.content, "content", "mp4", "Contents to parse and output, available values: \nbox-types: supported boxes(no parse) \nnalu-types: NALU types(no parse)  \navc_es: AVC elementary streams \nmp4: MP4 boxes")
	flag.StringVar(&flags.format, "format", "json", "Output format, available values:json,json_newlines,yaml,csv. \nNote that 'csv' only available for 'print-boxes'")
}

const (
	flagFormatJSON = iota
	flagFormatNewLines
	flagFormatYAML
	flagFormatCSV
)

func getFormatFlag() int {
	switch strings.ToLower(flags.format) {
	case "yaml":
		return flagFormatYAML
	case "json_newlines":
		return flagFormatNewLines
	case "csv":
		return flagFormatCSV
	}
	return flagFormatJSON
}

const (
	flagContentMP4   = iota // mp4 boxes
	flagContentAVCES        // AVC Elementary Streams

	// no parse needed
	flagContentBoxTypes
	flagContentNALUTypes
)

func getContentTypeFlag() int {
	switch strings.ToLower(flags.content) {
	case "avc_es":
		return flagContentAVCES

	case "box-types":
		return flagContentBoxTypes
	case "nalu-types":
		return flagContentNALUTypes
	}
	return flagContentMP4
}
