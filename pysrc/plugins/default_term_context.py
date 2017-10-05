from wxglterm_interface import Plugin, print_plugin_info, TermContext
from context_base import ContextBase

class DefaultTermContext(ContextBase, TermContext):
    def __init__(self):
        ContextBase.__init__(self, name="default_term_context",
                             desc="It is a python version term default_term_context",
                             version=1)
        TermContext.__init__(self)

def register_plugins(pm):
    ni = DefaultTermContext().new_instance()
    pm.register_plugin(ni)
