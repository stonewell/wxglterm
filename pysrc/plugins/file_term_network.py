from wxglterm_interface import Plugin, print_plugin_info, TermNetwork
from multiple_instance_plugin_base import MultipleInstancePluginBase

class FileTermNetwork(MultipleInstancePluginBase, TermNetwork):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self, name="term_network_use_file",
                                            desc="It is a python version term file_term_network",
                                            version=1)
        TermNetwork.__init__(self)

    def disconnect(self):
        pass

    def connect(self, host, port, user_name, password):
        file_path = self.plugin_config.get_entry("/file", "NOT FOUND");

        print("file_path:", file_path)

def register_plugins(pm):
    pm.register_plugin(FileTermNetwork().new_instance())
