from wxglterm_interface import Plugin, print_plugin_info, MultipleInstancePlugin
from term_plugin_base import TermPluginBase

g_instances = []

class MultipleInstancePluginBase(TermPluginBase, MultipleInstancePlugin):
    def __init__(self, name, desc, version):
        TermPluginBase.__init__(self, name=name,
                                desc=desc,
                                version=version)
        MultipleInstancePlugin.__init__(self)

    def new_instance(self):
        __new_instance = self.__class__()
        g_instances.append(__new_instance)

        return __new_instance
