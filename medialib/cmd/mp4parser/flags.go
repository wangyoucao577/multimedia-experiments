package main

import "flag"

var flags struct {
	inputFilePath string
}

func init() {
	flag.StringVar(&flags.inputFilePath, "i", "", "Input mp4 file path.")
}
