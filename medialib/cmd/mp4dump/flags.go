package main

import (
	"flag"
	"strings"
)

var flags struct {
	inputFilePath string
	format        string
	printBoxes    bool
}

func init() {
	flag.StringVar(&flags.inputFilePath, "i", "", "Input mp4 file path.")
	flag.StringVar(&flags.format, "format", "json", "Output format, available values:json,json_newlines,yaml,csv. \nNote that 'csv' only available for 'print-boxes'")
	flag.BoolVar(&flags.printBoxes, "print-boxes", false, "Print all supported boxes.")
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
