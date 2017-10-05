from wxglterm_interface import Plugin

class TermPluginBase(Plugin):
    def __init__(self, name, desc, version, **kw):
        Plugin.__init__(self)
        self._name = name
        self._desc = desc
        self._version = version

    def get_name(self):
        return self._name

    def get_description(self):
        return self._desc;

    def get_version(self):
        return self._version

    def init_plugin(self, context, plugin_config):
        self.context = context
        self.plugin_config = plugin_config
