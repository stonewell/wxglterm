import logging

from .term_char_width import char_width
from .charset_mode import translate_char, translate_char_british

LOGGER = logging.getLogger('TermBufferHandler')

class TermBufferHandler(object):
    def __init__(self):
        self.remain_buffer = []

    def __wrap_line(self, chars, insert):
        save_col, save_row = self.col, self.row

        self.col = 0
        self.cursor_down(None)
        for c in chars:
            if c == '\000':
                continue
            self.__save_buffer_2(c, insert)

        if insert:
            self.col, self.row = save_col, save_row

    def __save_buffer(self, c, insert = False):
        line = self.get_cur_line()

        #take care utf_8
        self.remain_buffer.append(c)

        c = ''.join(self.remain_buffer).decode('utf_8', errors='ignore')

        if len(c) == 0:
            if self.is_debug():
                LOGGER.debug('remain_buffer found:{}'.format(map(ord, self.remain_buffer)))
            return

        self.remain_buffer = []

        #translate g0, g1 charset
        c = self.__translate_char(c)

        w = char_width(c)

        if w == 0 or w == -1:
            LOGGER.warning(u'save buffer get a invalid width char: w= {}, c={}'.format(w, c))

        if len(c.encode('utf_8')) > 1 and w > 1:
            c += '\000'

        if self.is_debug():
            LOGGER.debug(u'save buffer width:{},{},{},len={}, line_len={}, cols={}'.format(self.col, self.row, w, len(c), line.cell_count(), self.get_cols()))

        if insert:
            if line.cell_count() + len(c) > self.get_cols():
                wrap_c = line[self.get_cols() - line.cell_count() - len(c):]

                if wrap_c[0].get_char() == '\000':
                    wrap_c = line[self.get_cols() - line.cell_count() - len(c) - 1:]

                two_bytes = len(wrap_c)

                line = line[:self.get_cols() - two_bytes]

                if self.is_debug():
                    LOGGER.debug(u'save buffer wrap:c=[{}], wrap=[{}]'.format(c, wrap_c))

                self.__save_buffer_2(c, insert)
                self.__wrap_line(''.join([c.get_char() for c in wrap_c]), insert)
            else:
                self.__save_buffer_2(c, insert)
        else:
            if self.col + len(c) > self.get_cols():
                #wrap
                self.__wrap_line(c, insert)
            else:
                self.__save_buffer_2(c, insert)


    def __save_buffer_2(self, c, insert):
        line = self.get_cur_line()

        if self.is_debug():
            LOGGER.debug(u'save buffer:{},{},{},len={}'.format(self.col, self.row, c, len(c)))

        if insert:
            line.insert_cell(self.col, Cell(c[0], self.cur_line_option, len(c) > 1))

            if len(c) > 1:
                line.insert_cell(self.col, Cell(c[1], self.cur_line_option, len(c) > 1))
        else:
            line.alloc_cells(self.col + len(c))
            if self.is_debug():
                LOGGER.debug(u'save buffer option:{},{},{},option={}'.format(self.col, self.row,
                                                                             c, self.cur_line_option))
            line.get_cell(self.col).set_char(c[0])
            line.get_cell(self.col).set_attr(self.cur_line_option)
            line.get_cell(self.col).set_is_wide_char(len(c) > 1)

            self.col += 1
            if len(c) > 1:
                line.get_cell(self.col).set_char(c[1])
                line.get_cell(self.col).set_attr(self.cur_line_option)
                line.get_cell(self.col).set_is_wide_char(len(c) > 1)
                self.col += 1
