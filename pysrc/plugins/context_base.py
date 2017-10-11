from wxglterm_interface import Plugin, print_plugin_info, Context
from multiple_instance_plugin_base import MultipleInstancePluginBase

class ContextBase(MultipleInstancePluginBase, Context):
    def __init__(self, name, desc, version):
        MultipleInstancePluginBase.__init__(self, name=name,
                                            desc=desc,
                                            version=version)
        Context.__init__(self)
        self._app_config = None

    def get_app_config(self):
        return self._app_config

    def set_app_config(self, app_config):
        self._app_config = app_config

    app_config = property(get_app_config, set_app_config)
