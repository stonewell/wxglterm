import os
import logging
import threading

from wxglterm_interface import TermCell, TermBuffer
from .charset_mode import translate_char, translate_char_british


#logging.getLogger('').setLevel(logging.DEBUG)
LOGGER = logging.getLogger('TermCapHandler')
#LOGGER.setLevel(logging.DEBUG)
TAB_MAX = 999


class TermCapHandler(object):
    def __init__(self):
        self._cur_cell = None
        self.charset_modes_translate = [None, None]
        self.charset_mode = 0

        self._dec_mode = False
        self._force_column = False
        self._force_column_count = 80

        self._origin_mode = False
        self._saved_origin_mode = False

        self._auto_wrap = False

        self._tab_stops = {}
        self._set_default_tab_stops()

        self._refresh_timer = self.__create_refresh_timer()

    def _set_default_tab_stops(self):
        tab_width = self.get_tab_width()

        for i in range(0, TAB_MAX, tab_width):
            self._tab_stops[i] = True

    def get_tab_width(self):
        return 8

    def resize_terminal(self):
        self.plugin_context.term_buffer.resize(self.get_rows(), self.get_cols())

        self.set_scroll_region(0, self.get_rows() - 1)

        if self.row >= self.get_rows():
            self.row = self.get_rows() - 1

        if self.col >= self.get_cols():
            self.col = self.get_cols() - 1

    @property
    def cur_cell(self):
        if not self._cur_cell:
            self._cur_cell = self.create_default_cell()
            self._default_cell = self.create_default_cell()

        return self._cur_cell

    @cur_cell.setter
    def cur_cell(self, c):
        self._cur_cell = c

    def create_default_cell(self):
        return self.plugin_context.term_buffer.create_cell_with_defaults()

    def get_rows(self):
        if self._force_column:
            return self._cap.flags['lines']
        return self.plugin_context.term_buffer.rows

    def get_cols(self):
        if self._force_column:
            return self._force_column_count
        return self.plugin_context.term_buffer.cols

    def set_cur_col(self, col):
        self.plugin_context.term_buffer.col = col

    def get_cur_col(self):
        return self.plugin_context.term_buffer.col

    col = property(get_cur_col, set_cur_col)

    def set_cur_row(self, row):
        self.plugin_context.term_buffer.row = row

    def get_cur_row(self):
        return self.plugin_context.term_buffer.row

    row = property(get_cur_row, set_cur_row)

    def get_cur_line(self):
        return self.plugin_context.term_buffer.cur_line

    def get_line(self, row):
        return self.plugin_context.term_buffer.get_line(row)

    def send_data(self, data):
        self.plugin_context.term_network.send(data, len(data))

    def get_cursor(self):
        return (self.col, self.row)

    def set_cursor(self, cursor):
        col, row = cursor

        end = self.get_rows() - 1

        if self._origin_mode:
            begin, end = self.get_scroll_region()
            row += begin

        if row > end:
            row = end

        if col > self.get_cols():
            col = self.get_cols() - 1

        self.set_cur_col(col)
        self.set_cur_row(row)

    cursor = property(get_cursor, set_cursor)

    def __create_refresh_timer(self, interval = 0.01):
        return threading.Timer(interval, self.__do_refresh)

    def __do_refresh(self):
        self.plugin_context.term_window.refresh()

    def refresh_display(self, interval=0.001):
        #self._refresh_timer.cancel()
        #self._refresh_timer = self.__create_refresh_timer(interval)
        #self._refresh_timer.start()
        self.plugin_context.term_window.refresh()

    def get_scroll_region(self):
        return (self.plugin_context.term_buffer.scroll_region_begin,
                self.plugin_context.term_buffer.scroll_region_end)

    def set_scroll_region(self, begin, end):
        (self.plugin_context.term_buffer.scroll_region_begin,
         self.plugin_context.term_buffer.scroll_region_end) = begin, end

    scroll_region = property(get_scroll_region, set_scroll_region)

    def is_debug(self):
        return False

    def create_new_buffer(self):
        new_buffer = self.plugin_context.term_buffer.new_instance()
        new_buffer.__class__ = TermBuffer

        new_buffer.resize(self.plugin_context.term_buffer.rows,
                          self.plugin_context.term_buffer.cols)
        return new_buffer;

    def get_line_cells(self, line):
        for col in range(self.get_cols()):
            yield line.get_cell(col)

    def cursor_right(self, context):
        self.parm_right_cursor(context)

    def cursor_left(self, context):
        self.parm_left_cursor(context)

    def cursor_down(self, context):
        self.parm_down_cursor(context, True, False)

    def cursor_up(self, context):
        self.parm_up_cursor(context, True, False)

    def carriage_return(self, context):
        self.set_cur_col(0)

    def set_foreground(self, light, color_idx):
        self.set_attributes([1] if light else [], color_idx, -2)

    def set_background(self, light, color_idx):
        self.set_attributes([1] if light else [], -2, color_idx)

    def origin_pair(self):
        self.cur_cell.fore_color_idx = self._default_cell.fore_color_idx
        self.cur_cell.back_color_idx = self._default_cell.back_color_idx

    def clr_line(self, context):
        line = self.get_cur_line()

        if not line:
            return

        for cell in self.get_line_cells(line):
            if not cell:
                continue
            cell.reset(self.cur_cell)

        self.refresh_display()

    def clr_eol(self, context):
        line = self.get_cur_line()

        if not line:
            return

        begin = self.col

        if begin < self.get_cols() and line.get_cell(begin).char == '\000':
            begin -= 1

        for i in range(begin, self.get_cols()):
            cell = line.get_cell(i)

            if cell:
                cell.reset(self.cur_cell)

        self.refresh_display()

    def clr_bol(self, context):
        line = self.get_cur_line()

        if not line:
            return

        end = self.col
        if end + 1 < self.get_cols() and line.get_cell(end + 1).char == '\000':
            end = end + 1

        for i in range(end + 1):
            cell = line.get_cell(i)

            if cell:
                cell.reset(self.cur_cell)

        self.refresh_display()

    def insert_chars(self, str):
        self._output_normal_data(str, True)

    def delete_chars(self, count, overwrite = False):
        line = self.get_cur_line()

        if not line:
            return

        begin = self.col

        if line.get_cell(begin).char == '\000':
            begin -= 1

        end = self.get_cols() if not overwrite or begin + count > self.get_cols() else begin + count

        for i in range(begin, end):
            if not overwrite and i + count < self.get_cols():
                line.get_cell(i).reset(line.get_cell(i + count))
            else:
                line.get_cell(i).reset(self.cur_cell)

        self.refresh_display()

    def meta_on(self, context):
        if self.is_debug():
            LOGGER.debug('meta_on')

    def set_attributes(self, mode, f_color_idx, b_color_idx):
        if len(mode) > 0:
            for m in mode:
                if m == 1:
                    self.cur_cell.add_mode(TermCell.TextMode.Bold)
                elif m == 2:
                    self.cur_cell.add_mode(TermCell.TextMode.Dim)
                elif m == 7:
                    self.cur_cell.add_mode(TermCell.TextMode.Reverse)
                elif m == 21 or m == 22:
                    self.cur_cell.remove_mode(TermCell.TextMode.Bold)
                    self.cur_cell.remove_mode(TermCell.TextMode.Dim)
                elif m == 27:
                    self.cur_cell.remove_mode(TermCell.TextMode.Reverse)
                elif m == 0:
                    self.cur_cell.mode = self._default_cell.mode
                    if self.is_debug():
                        LOGGER.debug('reset mode')
        else:
            self.cur_cell.mode = self._default_cell.mode
            if self.is_debug():
                LOGGER.debug('reset mode')

        if f_color_idx >= 0:
            self.cur_cell.fore_color_idx = f_color_idx
            if self.is_debug():
                LOGGER.debug('set fore color:{} {} {}, cur_option:{}'.format(f_color_idx, ' at ', self.get_cursor(), self.cur_cell))
        elif f_color_idx == -1:
            #reset fore color
            self.cur_cell.fore_color_idx = self._default_cell.fore_color_idx
            if self.is_debug():
                LOGGER.debug('reset fore color:{} {} {}, cur_option:{}'.format(f_color_idx, ' at ', self.get_cursor(), self.cur_cell))

        if b_color_idx >= 0:
            if self.is_debug():
                LOGGER.debug('set back color:{} {} {}, cur_option:{}'.format(b_color_idx, ' at ', self.get_cursor(), self.cur_cell))
            self.cur_cell.back_color_idx = b_color_idx
        elif b_color_idx == -1:
            #reset back color
            if self.is_debug():
                LOGGER.debug('reset back color:{} {} {}, cur_option:{}'.format(b_color_idx, ' at ', self.get_cursor(), self.cur_cell))
            self.cur_cell.back_color_idx = self._default_cell.back_color_idx

        if self.is_debug():
            LOGGER.debug('set attribute:{}'.format(self.cur_cell))

    def cursor_address(self, context):
        if self.is_debug():
            LOGGER.debug('cursor address:{}'.format(context.params))
        self.cursor = (context.params[1], context.params[0])

    def cursor_home(self, context):
        self.cursor = (0, 0)

    def clr_eos(self, context):
        begin = 0
        end = self.get_rows()

        if context:
            if len(context.params) == 0 or context.params[0] == 0:
                self.clr_eol(context)

                begin = self.row + 1
            elif context.params[0] == 1:
                self.clr_bol(context)

                end = self.row

        for row in range(begin, end):
            line = self.get_line(row)

            if not line:
                continue

            for cell in self.get_line_cells(line):
                if not cell:
                    continue
                cell.reset(self.cur_cell)

        self.refresh_display()

    def parm_right_cursor(self, context):
        #same as xterm, if cursor out of screen, moving start from last col
        col = self.col

        if col >= self.get_cols():
            col = self.get_cols() - 1

        col += context.params[0] if len(context.params) > 0 and context.params[0] > 0 else 1

        if col > self.get_cols():
            col = self.get_cols() - 1

        self.set_cur_col(col)

    def parm_left_cursor(self, context):
        #same as xterm, if cursor out of screen, moving start from last col
        col = self.col
        if col >= self.get_cols():
            col = self.get_cols() - 1

        col -= context.params[0] if len(context.params) > 0 and context.params[0] > 0 else 1
        if col < 0:
            col = 0

        self.set_cur_col(col)

    def client_report_version(self, context):
        self.send_data('\033[>0;136;0c')

    def user7(self, context):
        if (context.params[0] == 6):
            col, row = self.get_cursor()

            if self._origin_mode:
                begin, end = self.get_scroll_region()
                row -= begin

            self.send_data(''.join(['\x1B[', str(row + 1), ';', str(col + 1), 'R']))
        elif context.params[0] == 5:
            self.send_data('\033[0n')

    def clear_tab(self, context):
        action = 0
        if context.params and len(context.params) > 0:
            action = context.params[0]

        if action == 0:
            self._tab_stops.pop(self.col, 0)
        elif action == 3:
            self._tab_stops.clear()

    def clear_all_tabs(self, context):
        self._tab_stops.clear()

    def set_tab(self, context):
        self._tab_stops[self.col] = True

    def tab(self, context):
        col = self.col

        if len(self._tab_stops) > 0:
            for c in range(self.col+1, TAB_MAX):
                if c in self._tab_stops:
                    col = c
                    break

        if col >= self.get_cols():
            col = self.get_cols() - 1

        self.col = col
        self.refresh_display()

    def row_address(self, context):
        self.cursor = (self.col, context.params[0])

    def delete_line(self, context):
        self.parm_delete_line(context)

    def parm_delete_line(self, context):
        if self.is_debug():
            begin, end = self.get_scroll_region()
            LOGGER.debug('delete line:{} begin={} end={}, row={}'.format(context.params, begin, end, self.row))

        c_to_delete = context.params[0] if len(context.params) > 0 else 1

        self.plugin_context.term_buffer.delete_lines(self.row, c_to_delete)

        self.refresh_display()

    def change_scroll_region(self, context):
        if self.is_debug():
            LOGGER.debug('change scroll region:{} rows={}'.format(context.params, self.get_rows()))
        if len(context.params) == 0:
            self.set_scroll_region(0, self.get_rows() - 1)
        else:
            self.set_scroll_region(context.params[0], context.params[1])
        self.cursor_home(None)

    def change_scroll_region_from_start(self, context):
        if self.is_debug():
            LOGGER.debug('change scroll region from start:{} rows={}'.format(context.params, self.get_rows()))
        self.set_scroll_region(0, context.params[0])
        self.cursor_home(None)

    def change_scroll_region_to_end(self, context):
        if self.is_debug():
            LOGGER.debug('change scroll region to end:{} rows={}'.format(context.params, self.get_rows()))
        self.set_scroll_region(context.params[0], self.get_rows() - 1)
        self.cursor_home(None)

    def insert_line(self, context):
        self.parm_insert_line(context)

    def parm_insert_line(self, context):
        if self.is_debug():
            begin, end = self.get_scroll_region()
            LOGGER.debug('insert line:{} begin={} end={}, row={}'.format(context.params, begin, end, self.row))

        c_to_insert = context.params[0] if len(context.params) > 0 else 1

        self.plugin_context.term_buffer.insert_lines(self.row, c_to_insert)

        self.refresh_display()

    def request_background_color(self, context):
        rbg_response = '\033]11;rgb:%04x/%04x/%04x/%04x\007' % (self.cfg.default_background_color[0], self.cfg.default_background_color[1], self.cfg.default_background_color[2], self.cfg.default_background_color[3])

        if self.is_debug():
            LOGGER.debug("response background color request:{}".format(rbg_response.replace('\033', '\\E')))
        self.send_data(rbg_response)

    def user9(self, context):
        if self.is_debug():
            LOGGER.debug('response terminal type:{} {}'.format(context.params, self._cap_.cmds['user8'].cap_value))
        self.send_data(self._cap.cmds['user8'].cap_value)

    def enter_reverse_mode(self, context):
        self.cur_cell.add_mode(TermCell.TextMode.Reverse)
        self.refresh_display()

    def exit_standout_mode(self, context):
        self.cur_cell.mode = self._default_cell.mode

    def enter_ca_mode(self, context):
        self.savedcur_cell, self.cur_cell = \
            self.cur_cell, self.create_default_cell()

        self.plugin_context.term_buffer.enable_alter_buffer(True)

        self.refresh_display()

    def exit_ca_mode(self, context):
        self.cur_cell = self.savedcur_cell

        self.plugin_context.term_buffer.enable_alter_buffer(False)

        self.refresh_display()

    def key_shome(self, context):
        self.cursor = (1, 0)

    def enter_bold_mode(self, context):
        self.cur_cell.add_mode(TermCell.TextMode.Bold)
        if self.is_debug():
            LOGGER.debug('set bold mode:attr={}'.format(self.cur_cell))

    def keypad_xmit(self, context):
        if self.is_debug():
            LOGGER.debug('keypad transmit mode')
        self.keypad_transmit_mode = True

    def keypad_local(self, context):
        if self.is_debug():
            LOGGER.debug('keypad local mode')
        self.keypad_transmit_mode = False

    def cursor_invisible(self, context):
        self._cursor_visible = False

    def cursor_normal(self, context):
        self._cursor_visible = True

    def cursor_visible(self, context):
        self.cursor_normal(context)

    def next_line(self, context):
        self.col = 0
        self.parm_down_cursor(context, True, True)

    def line_feed(self, context):
        self.parm_down_cursor(context, True, True)

    def parm_index(self, context):
        saved_cursor = self.cursor
        self.parm_down_cursor(context, True, True)
        self.cursor = saved_cursor

    def parm_rindex(self, context):
        saved_cursor = self.cursor
        self.parm_up_cursor(context, True, True)
        self.cursor = saved_cursor

    def parm_down_cursor(self, context, do_refresh=True, do_scroll=True):
        begin, end = self.get_scroll_region()

        count = context.params[0] if context and context.params and len(context.params) > 0 else 1

        if self.is_debug():
            LOGGER.debug('before parm down cursor:{} {} {} {} {}'.format(begin, end, self.row, count, do_scroll))

        scrolled = self.plugin_context.term_buffer.move_cur_row(count, True, do_scroll)

        if self.is_debug():
            LOGGER.debug('after parm down cursor:{} {} {} {}'.format(begin, end, self.row, count))

        if do_refresh and scrolled:
            self.refresh_display()

    def exit_alt_charset_mode(self, context):
        self.charset_modes_translate[0] = None
        self.exit_standout_mode(context)
        if self.is_debug():
            LOGGER.debug('exit alt:{} {}'.format(' at ', self.get_cursor()))

    def enter_alt_charset_mode(self, context):
        self.charset_modes_translate[0] = translate_char
        if self.is_debug():
            LOGGER.debug('enter alt:{} {}'.format(' at ', self.get_cursor()))

    def enter_alt_charset_mode_british(self, context):
        self.charset_modes_translate[0] = translate_char_british

    def enter_alt_charset_mode_g1(self, context):
        self.charset_modes_translate[1] = translate_char

    def enter_alt_charset_mode_g1_british(self, context):
        self.charset_modes_translate[1] = translate_char_british

    def exit_alt_charset_mode_g1_british(self, context):
        self.charset_modes_translate[1] = None
        self.exit_standout_mode(context)

    def shift_in_to_charset_mode_g0(self, context):
        self.charset_mode = 0

    def shift_out_to_charset_mode_g1(self, context):
        self.charset_mode = 1

    def enable_mode(self, context):
        if self.is_debug():
            LOGGER.debug('enable mode:{}'.format(context.params))

        mode = context.params[0]

        if mode == 25:
            self.cursor_normal(context)
        elif mode == 40:
            self._dec_mode = True
            self._force_column = True
            self.resize_terminal()
        elif mode == 3:
            if self._dec_mode:
                self._force_column = True
                self._force_column_count = 132
                self.resize_terminal()

                self.clr_eos(None)
                self.cursor_home(None)
        elif mode == 5:
            self.cur_cell.add_mode(TermCell.TextMode.Reverse)
        elif mode == 6:
            self._origin_mode = True
            self.cursor_home(None)
        elif mode == 7:
            self._auto_wrap = True
        else:
            LOGGER.warning('not implemented enable mode:{}'.format(context.params))

    def disable_mode(self, context):
        if self.is_debug():
            LOGGER.debug('disable mode:{}'.format(context.params))

        mode = context.params[0]

        if mode == 25:
            self.cursor_invisible(context)
        elif mode == 40:
            self._dec_mode = False
            self._force_column = False
            self.resize_terminal()
        elif mode == 3:
            if self._dec_mode:
                self._force_column = True
                self._force_column_count = 80
                self.resize_terminal()

                self.clr_eos(None)
                self.cursor_home(None)
        elif mode == 5:
            self.cur_cell.remove_mode(TermCell.TextMode.Reverse)
        elif mode == 6:
            self._origin_mode = False
            self.cursor_home(None)
        elif mode == 7:
            self._auto_wrap = False
        else:
            LOGGER.warning('not implemented disable mode:{}'.format(context.params))

    def column_address(self, context):
        col, row = self.get_cursor()
        self.cursor = (context.params[0], row)

    def parm_up_cursor(self, context, do_refresh = True, do_scroll = True):
        begin, end = self.get_scroll_region()

        count = context.params[0] if context and context.params and len(context.params) > 0 else 1

        if self.is_debug():
            LOGGER.debug('before parm up cursor:{} {} {} {}'.format(begin, end, self.row, count))

        scrolled = self.plugin_context.term_buffer.move_cur_row(count, False, do_scroll)

        if self.is_debug():
            LOGGER.debug('after parm up cursor:{} {} {} {}'.format(begin, end, self.row, count))
        if do_refresh and scrolled:
            self.refresh_display()

    def send_primary_device_attributes(self, context):
        self.send_data('\033[?62;c')

    def screen_alignment_test(self, context):
        self.save_cursor(context)

        for i in range(self.get_rows()):
            line = self.get_line(i)

            if not line:
                continue

            for cell in self.get_line_cells(line):
                if not cell:
                    continue
                cell.char = 'E'

        self.restore_cursor(context)
        self.refresh_display()

    def enter_status_line(self, mode, enter):
        self.in_status_line = enter

    def _translate_char(self, c):
        if self.charset_modes_translate[self.charset_mode]:
            return self.charset_modes_translate[self.charset_mode](c)
        else:
            return c

    def save_cursor(self, context):
        self._saved_cursor = self.cursor

    def restore_cursor(self, context):
        self.cursor = self._saved_cursor
