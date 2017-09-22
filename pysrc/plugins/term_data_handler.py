from wxglterm_interface import Plugin, print_plugin_info
from term_plugin_base import TermPluginBase
from utils.term_utils import test_util

class TermDataHandler(TermPluginBase):
    def __init__(self):
        TermPluginBase.__init__(self, name="TermDataHandler",
                         desc="It is a python version term data handler",
                         version=1)


g_term_data_handler = TermDataHandler()

print_plugin_info(g_term_data_handler)
test_util()

def register_plugins(pm):
    pm.register_plugin(g_term_data_handler)
