#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"

/*
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
    def insert_line(self, context):
        self.parm_insert_line(context)

    def parm_insert_line(self, context):
        if self.is_debug():
            begin, end = self.get_scroll_region()
            LOGGER.debug('insert line:{} begin={} end={}, row={}'.format(context.params, begin, end, self.row))

        c_to_insert = context.params[0] if len(context.params) > 0 else 1

        self.plugin_context.term_buffer.insert_lines(self.row, c_to_insert)

        self.refresh_display()

    def delete_line(self, context):
        self.parm_delete_line(context)

    def parm_delete_line(self, context):
        if self.is_debug():
            begin, end = self.get_scroll_region()
            LOGGER.debug('delete line:{} begin={} end={}, row={}'.format(context.params, begin, end, self.row))

        c_to_delete = context.params[0] if len(context.params) > 0 else 1

        self.plugin_context.term_buffer.delete_lines(self.row, c_to_delete)

        self.refresh_display()
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
            */
