"""
Script that produces cube data for pasting in C script.
Usage: python cube.py <height> <width> <depth> > xclip
"""

import sys

def output_rectangle(dim1, dim2, str_format):

    half1 = dim1/2.0
    half2 = dim2/2.0

    return [
        str_format.format(-half1, -half2),
        str_format.format( half1, -half2),
        str_format.format( half1,  half2),
        str_format.format(-half1, -half2),
        str_format.format( half1,  half2),
        str_format.format(-half1,  half2),
    ]

def format_string(coord):
    tokens = []
    for x in range(3):
        if (x == coord):
            tokens.append("{:>6}f")
        else:
            tokens.append("{{:>6}}f")
    return '"{},\\n"'.format(', '.join(tokens))


def generate_output(height, width, depth):

    sides = []
    sides += output_rectangle(height, width, format_string(2).format(-depth/2.0))
    sides += output_rectangle(height, width, format_string(2).format(+depth/2.0))
    sides += output_rectangle(width, depth, format_string(0).format(-height/2.0))
    sides += output_rectangle(width, depth, format_string(0).format(+height/2.0))
    sides += output_rectangle(height, depth, format_string(1).format(-width/2.0))
    sides += output_rectangle(height, depth, format_string(1).format(+width/2.0))
    sides[-1] = sides[-1]+";"
    for line in sides:
        print(line)


def main(args):
    generate_output(*args)

if __name__ == "__main__":
    main([float(var) for var in sys.argv[1:]])

