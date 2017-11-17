import logging

from .term_char_width import char_width
from .charset_mode import translate_char, translate_char_british

LOGGER = logging.getLogger('TermBufferHandler')

class TermBufferHandler(object):
    def __init__(self):
        self.remain_buffer = []

    def _save_buffer(self, c, insert = False):
        line = self.get_cur_line()

        #take care utf_8
        self.remain_buffer.append(c)

        c = bytes(''.join(self.remain_buffer),'iso8859-1').decode('utf_8', errors='ignore')

        if len(c) == 0:
            if self.is_debug():
                LOGGER.debug('remain_buffer found:{}'.format(map(ord, self.remain_buffer)))
            return

        self.remain_buffer = []

        #translate g0, g1 charset
        c = self._translate_char(c)

        w = char_width(c)

        if w == 0 or w == -1:
            LOGGER.warning(u'save buffer get a invalid width char: w= {}, c={},{}'.format(w, c, ord(c)))

        wide_char = False
        if len(c.encode('utf_8')) > 1 and w > 1:
            wide_char = True

        if self.is_debug():
            LOGGER.debug(u'save buffer width:{},{},{},len={}, line_len={}, cols={}'.format(self.col, self.row, w, len(c), 1, self.get_cols()))

        if not self._auto_wrap and self.col >= self.get_cols():
            self.col = self.get_cols() - 1

        self.get_plugin_context().term_buffer.set_cur_cell_data(ord(c),
                                                          wide_char,
                                                          insert,
                                                          self.cur_cell)
