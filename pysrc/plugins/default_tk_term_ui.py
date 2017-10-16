from wxglterm_interface import Plugin, print_plugin_info, TermUI, TermWindow
from term_plugin_base import TermPluginBase
from utils.term_utils import test_util

import tkinter
import sys
import logging

class DefaultTkTermWindow(TermPluginBase, TermWindow):
    def __init__(self):
        TermPluginBase.__init__(self,
                            name="default_term_window",
                            desc="It is a python version term default_term_ui",
                            version=1)
        TermWindow.__init__(self)
        self.top = None

    def refresh(self):
        rows = self.plugin_context.term_buffer.rows
        cols = self.plugin_context.term_buffer.cols

        term_buff = self.plugin_context.term_buffer

        for row in range(rows):
            line = term_buff.get_line(row)

            data = []
            for col in range(cols):
                cell = line.get_cell(col)
                try:
                    if cell.char != 0:
                        data.append(cell.char)
                except:
                    logging.exception("char error")
                    sys.exit(1)
                    pass

            print(u''.join(data))


    def show(self):
        self.top = tkinter.Tk()

class DefaultTkTermUI(TermPluginBase, TermUI):
    def __init__(self):
        TermPluginBase.__init__(self,
                            name="default_term_ui",
                            desc="It is a python version term default_term_ui",
                            version=1)
        TermUI.__init__(self)

        if not hasattr(sys, 'argv') or len(sys.argv) == 0:
            sys.argv = ['']

        self.top = None
        self._windows = []

    def create_window(self):
        w = DefaultTkTermWindow()

        w.init_plugin(self.plugin_context,
                      self.plugin_config)
        self._windows.append(w)
        return w

    def start_main_ui_loop(self):
        if not self.top:
            self.top = tkinter.Tk()

        self.top.mainloop()

        return 0;

    def schedule_task(self, task, miliseconds, repeated):
        pass

g_term_ui = DefaultTkTermUI()

def register_plugins(pm):
    pm.register_plugin(g_term_ui)
