#convert a python file to embeded python string
import sys
import logging
import argparse

def do_convert(i_f, o_f):
    for line in i_f:
        line = line.translate(str.maketrans({
            '\\': r'\\',
            '"' : r'\"',
            '\n' : '',
            '\r' : ''
        }))
        line = ''.join(['"',
                       line,
                       '\\n',
                       '"'])
        o_f.write(line)
        o_f.write('\n')


def args_parser():
    parser = argparse.ArgumentParser(prog='pystmt_generator',
                                     description='convert a python file to embeded python string')
    parser.add_argument('-i', '--input', type=argparse.FileType('r'), help='source file to convert', required = True)
    parser.add_argument('-o', '--output', type=argparse.FileType('w'), help='destination file to write', required = True)

    return parser

if __name__ == '__main__':
    logging.getLogger('').setLevel(logging.INFO)
    args = args_parser().parse_args()
    do_convert(args.input, args.output)
