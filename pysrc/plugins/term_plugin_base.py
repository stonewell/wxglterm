from wxglterm_interface import Plugin

class TermPluginBase(Plugin):
    def __init__(self, name, desc, version, **kw):
        Plugin.__init__(self)
        self._name = name
        self._desc = desc
        self._version = version
        self._plugin_context = None
        self._plugin_config = None

    def get_name(self):
        return self._name

    def get_description(self):
        return self._desc;

    def get_version(self):
        return self._version

    def init_plugin(self, context, plugin_config):
        self._plugin_context = context
        self._plugin_config = plugin_config

    def get_plugin_context(self):
        return self._plugin_context

    def get_plugin_config(self):
        return self._plugin_config

    @property
    def plugin_context(self):
        return self._plugin_context

    @property
    def plugin_config(self):
        return self._plugin_config
