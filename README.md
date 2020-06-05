# translate
# A cpp version of the python program in the Chris repo.
## Overview:
This is a c++ program that reads a text file and writes a text file with a name derived from the name of the input file (e.g. d.txt -> d.OUT.txt). It reads the first data set, determines where the protein sequence is in the count. Then, in every subsequent data set,  the program will only return data that is related to that count. This will replace data that does that does not precisely align with sequence positions from the first data set (whether they are a dash or a letter). The output file will have the same number of data sets  as the input file. The protiens sequence of the first data set is unchanged. In the remaining data sets, each character of the sequence is unchanged if the corresponding character of the first sequence is a letter. Otherwise, it is replaced with the specified replacement character. If none is specified, the default '+' will be used.

## Input file format:
    '>' + other species information + '[' + full species name + ']' + protiens sequence

## Output file format:
    '>' + full species name + '\n' + protiens sequence + '\n'

## Usage:
    $./translate -l 77 -r + -o OUT <At least one file name>

## Examples:
1. translate some_file.txt -l 42 -r 0 -- line length = 42, replacement character = 0
1. translate -l 123 -r # some_file.txt another_file.txt -- line length = 123, replacement character = #
1. translate some_file.txt -- default line length (60), default replacement character (+)
