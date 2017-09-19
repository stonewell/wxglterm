from wxglterm_interface import Plugin, TermBuffer, print_plugin_info

class PluginBase(Plugin):
    def __init__(self, name, desc, version):
        super(PluginBase, self).__init__()
        self._name = name
        self._desc = desc
        self._version = version

    def get_name(self):
        return self._name

    def get_description(self):
        return self._desc;

    def get_version(self):
        return self._version


class MyTermBuffer(PluginBase, TermBuffer):
    def __init__(self):
        PluginBase.__init__(self, name="MyTermBuffer",
                         desc="It is a python test",
                         version=1)

    def get_name(self):
        return "hahahahhaha"

my_term_buffer = MyTermBuffer()
print(my_term_buffer.get_name())

print_plugin_info(MyTermBuffer())
