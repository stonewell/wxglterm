from wxglterm_interface import Plugin, TermBuffer, print_plugin_info, TermNetwork

class MyPluginBase(Plugin):
    def __init__(self, name, desc, version, **kw):
        Plugin.__init__(self)
        self._name = name
        self._desc = desc
        self._version = version
        print("1", kw)

    def get_name(self):
        return self._name

    def get_description(self):
        return self._desc;

    def get_version(self):
        return self._version

class MixinTermBuffer(TermNetwork):
    def __init__(self, **kw):
        TermNetwork.__init__(self)
        print("2", kw)

    def disconnect(self):
        print("disconnect")

class MyTermBuffer(MixinTermBuffer, MyPluginBase):
    def __init__(self):
        MixinTermBuffer.__init__(self)
        MyPluginBase.__init__(self, name="MyTermBuffer",
                         desc="It is a python test",
                         version=1)


my_term_buffer = MyTermBuffer()
print(my_term_buffer.get_name())

print_plugin_info(my_term_buffer)
