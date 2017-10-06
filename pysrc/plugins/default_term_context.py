from wxglterm_interface import Plugin, print_plugin_info, TermContext
from context_base import ContextBase

class DefaultTermContext(ContextBase, TermContext):
    def __init__(self):
        ContextBase.__init__(self, name="default_term_context",
                             desc="It is a python version term default_term_context",
                             version=1)
        TermContext.__init__(self)

        self.term_buffer = None
        self.term_ui = None
        self.term_network = None

    def get_term_buffer(self):
        return self.term_buffer

    def set_term_buffer(self, term_buffer):
        self.term_buffer = term_buffer

    def get_term_ui(self):
        return self.term_ui

    def set_term_ui(self, term_ui):
        self.term_ui = term_ui

    def get_term_network(self):
        return self.term_network

    def set_term_network(self, term_network):
        self.term_network = term_network

def register_plugins(pm):
    ni = DefaultTermContext().new_instance()
    pm.register_plugin(ni)
