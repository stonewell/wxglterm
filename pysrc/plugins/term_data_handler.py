from wxglterm_interface import Plugin, print_plugin_info, TermDataHandler
from multiple_instance_plugin_base import MultipleInstancePluginBase
from utils.term_utils import test_util

class DefaultTermDataHandler(MultipleInstancePluginBase, TermDataHandler):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self, name="default_term_data_handler",
                         desc="It is a python version term data handler",
                         version=1)
        TermDataHandler.__init__(self)


def register_plugins(pm):
    pm.register_plugin(DefaultTermDataHandler().new_instance())
