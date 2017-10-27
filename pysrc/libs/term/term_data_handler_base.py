import logging
import os
from collections import deque

import term.parse_termdata
import term.read_termdata

LOGGER = logging.getLogger('TermDataHandler')


class TermDataHandlerBase():
    def __init__(self):
        pass

    def init_with_term_name(self, termcap_dir, term_name):
        # initialize term caps
        self._term_name = term_name

        self._cap_str = self.__load_cap_str__(termcap_dir, 'generic-cap')
        try:
            self._cap_str += \
                self.__load_cap_str__(termcap_dir, self._term_name)
        except:
            LOGGER.exception('unable to load term data:%s' % self._term_name)
            self._cap_str += \
                self.__load_cap_str__(termcap_dir, 'xterm-256color')

        self._cap = term.parse_termdata.parse_cap(self._cap_str)

        self._parse_context = term.parse_termdata.ControlDataParserContext()
        self._state = self._cap.control_data_start_state
        self._control_data = []
        self._cap_state_stack = deque()

    def __load_cap_str__(self, termcap_dir, term_name):
        term_path = os.path.join(termcap_dir, term_name+'.dat')

        LOGGER.info('load term cap data file:{}'.format(term_path))

        return term.read_termdata.get_entry(term_path, term_name)

    def __handle_cap__(self, check_unknown=True, data=None, c=None):
        cap_turple = self._state.get_cap(self._parse_context.params)
        params = []

        if cap_turple:
            params = self._parse_context.params[:] if self._parse_context.params else []

            if len(self._cap_state_stack) > 0:
                (self._state,
                 self._parse_context.params,
                 self._control_data) = self._cap_state_stack.pop()
            else:
                self._state = self._cap.control_data_start_state
                self._parse_context.params = []
                self._control_data = []
        elif check_unknown and len(self._control_data) > 0:
            next_state = \
                self._cap.control_data_start_state.handle(self._parse_context,
                                                         c)

            if not next_state:
                m1 = 'start state:{}, params={}, self={}, next_states={}'.format(self._cap.control_data_start_state.cap_name, self._parse_context.params, self, self._cap.control_data_start_state.next_states)
                m2 = 'current state:{}, {}, params={}, next_states={}, {}, [{}]'.format(self._state, self._state.cap_name, self._parse_context.params, self._state.next_states, self._state.digit_state, ord(c) if c else 'None')
                m3 = "unknown control data:[[[" + ''.join(self._control_data) + ']]]'
                m4 = 'data:[[[{}]]]'.format(str(data).replace('\x1B', '\\E')
                                        .replace('\r', '\r\n'))
                m5 = 'data:[[[{}]]]]'.format(' '.join(map(str, map(ord, str(data)))))

                LOGGER.error('\r\n'.join([m1, m2, m3, m4, m5]))

            self._cap_state_stack.append((self._state,
                                          self._parse_context.params,
                                          self._control_data))

            self._state = self._cap.control_data_start_state
            self._parse_context.params = []
            self._control_data = []

        if not check_unknown and not cap_turple and len(self._control_data) > 0:
            LOGGER.debug('found unfinished data')

        return (cap_turple, params)

    def process_data(self, c=None):
        cap_turple = None
        char_to_output = None
        params = None

        if c is None:
            if self._state:
                cap_turple, params = self.__handle_cap__(False)
            return (cap_turple, params, char_to_output)

        next_state = self._state.handle(self._parse_context, c)

        if (not next_state or
                self._state.get_cap(self._parse_context.params)):
            cap_turple, params = self.__handle_cap__(c=c)

            # reset next state, if have both next_state
            # and cap, next_state may process digit value
            if next_state:
                next_state.reset()
            # retry last char
            next_state = self._state.handle(self._parse_context, c)

            if next_state:
                self._state = next_state
                self._control_data.append(c if not c == '\x1B' else '\\E')
            else:
                char_to_output = c

            return (cap_turple, params, char_to_output)

        self._state = next_state
        self._control_data.append(c if not c == '\x1B' else '\\E')

        return (cap_turple, params, char_to_output)
