from wxglterm_interface import TermUI, TermWindow
from term_plugin_base import TermPluginBase

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
        self.old_content = ''

    def refresh(self):
        if self.top:
            self.top.after(20, self.__refresh)
            pass

    def __refresh(self):
        rows = self.plugin_context.term_buffer.rows
        cols = self.plugin_context.term_buffer.cols

        term_buff = self.plugin_context.term_buffer

        self.text.delete('1.0', tkinter.END)
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

            self.text.insert('{}.{}'.format(row + 1, 0),
                             u''.join(data) + u'\n')

    def show(self):
        if not self.top:
            self.top = tkinter.Tk()
            self.text = tkinter.Text(self.top)
            self.text.pack(fill=tkinter.BOTH, expand=1)


class DefaultTkTermUI(TermPluginBase, TermUI):
    def __init__(self):
        TermPluginBase.__init__(self,
                            name="default_term_ui",
                            desc="It is a python version term default_term_ui",
                            version=1)
        TermUI.__init__(self)

        if not hasattr(sys, 'argv') or len(sys.argv) == 0:
            sys.argv = ['']

        self.top = tkinter.Tk()
        self.root_window = None
        self._windows = []

    def create_window(self):
        w = DefaultTkTermWindow()

        w.init_plugin(self.plugin_context,
                      self.plugin_config)

        if not self.root_window:
            w.top = self.top
            w.text = tkinter.Text(self.top)
            w.text.pack(fill=tkinter.BOTH, expand=1)
            self.root_window = w

        self._windows.append(w)
        return w

    def start_main_ui_loop(self):
        self.top.mainloop()

        return 0

    def schedule_task(self, task, miliseconds, repeated):
        self.top.after(miliseconds, lambda:task.run())
        pass

g_term_ui = DefaultTkTermUI()

def register_plugins(pm):
    pm.register_plugin(g_term_ui)
