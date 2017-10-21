import argparse
import os
import logging
import plistlib
import json


def args_parser():
    parser = argparse.ArgumentParser(prog='pyitemtheme_converter',
                                     description='convert a iterm color theme to wxglterm theme json')
    parser.add_argument('-i', '--input', type=argparse.FileType('r'), help='source file to convert', required = True)
    parser.add_argument('-o', '--output', type=argparse.FileType('w'), help='destination file to write', required = True)

    return parser


def convert_color(c):
    r = c['Red Component']
    g = c['Green Component']
    b = c['Blue Component']
    a = c['Alpha Component']

    result = [int(float(x) * 255) & 0xFF for x in [r, g, b, a]]
    result = map(lambda x: hex(x).replace('0x', ''), result)
    result = map(lambda x: x if len(x) == 2 else '0' + x, result)

    return reduce(lambda a, b: a + b, result)


def update_color_map(colors, i, p, key):
    try:
        c = p[key]

        colors[i] = convert_color(c)
    except:
        logging.exception('error')
        logging.error('no color:{} found'.format(key))


def do_convert(i_f_p, o_f_p):
    colors = {}

    p = plistlib.readPlist(i_f_p)
    for i in range(16):
        key = 'Ansi {} Color'.format(i)

        update_color_map(colors, i, p, key)

    update_color_map(colors, 256, p, 'Foreground Color')
    update_color_map(colors, 257, p, 'Background Color')
    update_color_map(colors, 258, p, 'Cursor Color')

    logging.info(json.dumps(colors, indent=4))

    json.dump(colors, o_f_p, indent=4)


if __name__ == '__main__':
    logging.getLogger('').setLevel(logging.INFO)
    args = args_parser().parse_args()
    do_convert(args.input, args.output)
