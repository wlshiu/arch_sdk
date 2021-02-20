#!/usr/bin/env python

import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", help="input file", type = str)
parser.add_argument("-o", "--output", help="output file", type = str)
parser.add_argument("-t", "--offset", help="offset of input (hex)", type = str)
parser.add_argument("-s", "--length", help="extracting length (hex)", type = str)
args = parser.parse_args()
# print args.input
# print args.output
# print args.offset
# print args.length


offset=int(args.offset, 16)
length=int(args.length, 16)
file_in = open(args.input, "rb")
file_in.seek(offset, 0)

file_out = open(args.output, "wb")
data = file_in.read(length)
file_out.write(data)

file_in.close()
file_out.close()