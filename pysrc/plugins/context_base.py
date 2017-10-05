from wxglterm_interface import Plugin, print_plugin_info, Context
from multiple_instance_plugin_base import MultipleInstancePluginBase

class ContextBase(MultipleInstancePluginBase, Context):
    def __init__(self, name, desc, version):
        MultipleInstancePluginBase.__init__(self, name=name,
                                            desc=desc,
                                            version=version)
        Context.__init__(self)
