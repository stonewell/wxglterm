from wxglterm_interface import Plugin, print_plugin_info, TermUI
from multiple_instance_plugin_base import MultipleInstancePluginBase
from utils.term_utils import test_util

import tkinter
import sys

class DefaultTkTermUI(MultipleInstancePluginBase, TermUI):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self, name="default_term_ui",
                                            desc="It is a python version term default_term_ui",
                                            version=1)
        TermUI.__init__(self)

        if not hasattr(sys, 'argv') or len(sys.argv) == 0:
            sys.argv = ['']

        self.top = None

    def refresh(self):
        pass

    def show(self):
        self.top = tkinter.Tk()

    def start_main_ui_loop(self):
        if not self.top:
            self.top = tkinter.Tk()

        self.top.mainloop()

        return 0;

def register_plugins(pm):
    pm.register_plugin(DefaultTkTermUI().new_instance())
