from wxglterm_interface import Plugin, print_plugin_info, TermUI
from term_plugin_base import TermPluginBase
from utils.term_utils import test_util

import tkinter
import sys

g_term_ui = []

class DefaultTkTermUI(TermPluginBase, TermUI):
    def __init__(self):
        TermPluginBase.__init__(self, name="default_term_ui",
                         desc="It is a python version term default_term_ui",
                         version=1)
        TermUI.__init__(self)

        sys.argv = ['']

        print(dir(TermUI))
        print(dir(self))

    def new_instance(self):
        print("default tk term ui new instance")
        new_instance = DefaultTkTermUI()
        g_term_ui.append(new_instance)

        return new_instance

    def refresh(self):
        pass

    def show(self):
        self.top = tkinter.Tk()
        self.top.mainloop()

def register_plugins(pm):
    g_term_ui.append(DefaultTkTermUI())
    pm.register_plugin(g_term_ui[0])
