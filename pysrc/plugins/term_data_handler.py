import logging
import os
import sys

from collections import deque

from wxglterm_interface import TermDataHandler
from multiple_instance_plugin_base import MultipleInstancePluginBase

import cap.cap_manager
import term.parse_termdata
import term.read_termdata

from term.term_cap_handler import TermCapHandler
from term.term_buffer_handler import TermBufferHandler

LOGGER = logging.getLogger('TermDataHandler')


class DefaultTermDataHandler(MultipleInstancePluginBase,
                             TermDataHandler,
                             TermCapHandler,
                             TermBufferHandler):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self, name="default_term_data_handler",
                                            desc="It is a python version term data handler",
                                            version=1)
        TermDataHandler.__init__(self)
        TermCapHandler.__init__(self)
        TermBufferHandler.__init__(self)

    def init_plugin(self, context, plugin_config):
        MultipleInstancePluginBase.init_plugin(self, context, plugin_config)

        #initialize term caps
        self._term_name = self.plugin_context.get_app_config().get_entry("/term/term_name", "xterm-256color")

        self.cap_str = self.__load_cap_str__('generic-cap')
        try:
            self.cap_str += self.__load_cap_str__(self._term_name)
        except:
            LOGGER.exception('unable to load term data:%s' % self._term_name)
            self.cap_str += self.__load_cap_str__('xterm-256color')

        self.cap = term.parse_termdata.parse_cap(self.cap_str)

        self._parse_context = term.parse_termdata.ControlDataParserContext()
        self.state = self.cap.control_data_start_state
        self.control_data = []
        self.in_status_line = False
        self.keypad_transmit_mode = False
        self._cap_state_stack = deque()

    def __load_cap_str__(self, term_name):
        termcap_dir = self.plugin_context.get_app_config().get_entry('/term/termcap_dir', 'data')
        term_path = os.path.join(termcap_dir, term_name+'.dat')

        LOGGER.info('load term cap data file:{}'.format(term_path))

        return term.read_termdata.get_entry(term_path, term_name)

    def on_data(self, data):
        self.__try_parse__(data)

    def __on_control_data(self, cap_turple):
        cap_name, increase_params = cap_turple
        cap_handler = cap.cap_manager.get_cap_handler(cap_name)

        LOGGER.debug("control data:[[[" + ''.join(self.control_data) + ']]],for cap:' + cap_name)
        if not cap_handler:
            LOGGER.error('no matched:{}, params={}'.format(cap_turple, self._parse_context.params))
        elif cap_handler:
            cap_handler.handle(self, self._parse_context, cap_turple)

    def __output_data(self, c):
        if self.in_status_line:
            self.__output_status_line_data(c)
        else:
            self.__output_normal_data(c)

    def __output_status_line_data(self, c):
        pass

    def __output_normal_data(self, c, insert = False):
        if c == '\x1b':
            LOGGER.error('normal data has escape char')
            sys.exit(1)

        try:
            for cc in c:
                self._save_buffer(cc, insert)
        except:
            LOGGER.exception('save buffer failed')

    def __handle_cap__(self, check_unknown = True, data = None, c = None):
        cap_turple = self.state.get_cap(self._parse_context.params)

        if cap_turple:
            self.__on_control_data(cap_turple)

            if len(self._cap_state_stack) > 0:
                self.state, self._parse_context.params, self.control_data = self._cap_state_stack.pop()
            else:
                self.state = self.cap.control_data_start_state
                self._parse_context.params = []
                self.control_data = []
        elif check_unknown and len(self.control_data) > 0:
            next_state = self.cap.control_data_start_state.handle(self._parse_context, c)

            if not next_state:
                m1 = 'start state:{}, params={}, self={}, next_states={}'.format(self.cap.control_data_start_state.cap_name, self._parse_context.params, self, self.cap.control_data_start_state.next_states)
                m2 = 'current state:{}, params={}, next_states={}, {}, [{}]'.format(self.state.cap_name, self._parse_context.params, self.state.next_states, self.state.digit_state, ord(c) if c else 'None')
                m3 = "unknown control data:[[[" + ''.join(self.control_data) + ']]]'
                m4 = 'data:[[[{}]]]'.format(str(data).replace('\x1B', '\\E')
                                        .replace('\r', '\r\n'))
                m5 = 'data:[[[{}]]]]'.format(' '.join(map(str, map(ord, str(data)))))

                LOGGER.error('\r\n'.join([m1, m2, m3, m4, m5, str(self.in_status_line)]))

            self._cap_state_stack.append((self.state, self._parse_context.params, self.control_data))

            self.state = self.cap.control_data_start_state
            self._parse_context.params = []
            self.control_data = []

        if not check_unknown and not cap_turple and len(self.control_data) > 0:
            LOGGER.debug('found unfinished data')

        return cap_turple

    def __try_parse__(self, data):
        next_state = None

        for c in data:
            c = chr(c)
            next_state = self.state.handle(self._parse_context, c)

            if not next_state or self.state.get_cap(self._parse_context.params):
                cap_turple = self.__handle_cap__(data=data, c=c)

                # retry last char
                next_state = self.state.handle(self._parse_context, c)

                if next_state:
                    self.state = next_state
                    self.control_data.append(c if not c == '\x1B' else '\\E')
                else:
                    self.__output_data(c)

                continue

            self.state = next_state
            self.control_data.append(c if not c == '\x1B' else '\\E')

        if self.state:
	        self.__handle_cap__(False)

def register_plugins(pm):
    pm.register_plugin(DefaultTermDataHandler().new_instance())
