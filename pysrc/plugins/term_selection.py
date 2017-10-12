from wxglterm_interface import TermSelection
from term_plugin_base import TermPluginBase


class DefaultTermSelection(TermPluginBase, TermSelection):
    def __init__(self):
        TermPluginBase.__init__(self,
                                name="default_term_selection_py",
                                desc="It is a python version term default_term_selection",
                                version=1)
        TermSelection.__init__(self)
        self._row_begin = 0
        self._row_end = 0
        self._col_begin = 0
        self._col_end = 0

    def get_row_begin(self):
        return self._row_begin

    def set_row_begin(self, r):
        self._row_begin = r

    def get_row_end(self):
        return self._row_end

    def set_row_end(self, r):
        self._row_end = r

    def get_col_begin(self):
        return self._col_begin

    def set_col_begin(self, r):
        self._col_begin = r

    def get_col_end(self):
        return self._col_end

    def set_col_end(self, r):
        self._col_end = r

    row_begin = property(get_row_begin, set_row_begin)
    row_end = property(get_row_end, set_row_end)
    col_begin = property(get_col_begin, set_col_begin)
    col_end = property(get_col_end, set_col_end)
