from term.term_data_handler_base import TermDataHandlerBase


class __TermDataHandler(TermDataHandlerBase):
    def __init__(self):
        TermDataHandlerBase.__init__(self)

    def on_term_data(self, c=None):
        if not c:
            print('************ handle None char', c)
        cap_turple, self.params, self.output_char = self.process_data(c)

        self.cap_name = None
        self.increase_param = False

        if cap_turple:
            self.cap_name, self.increase_param = cap_turple


def create_data_handler(termcap_dir, term_name):
    handler = __TermDataHandler()
    handler.init_with_term_name(termcap_dir, term_name)

    return handler
