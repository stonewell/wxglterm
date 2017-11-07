import logging
import os
import sys

from collections import deque
import threading
from threading import Event
from threading import Thread

from wxglterm_interface import TermDataHandler
from multiple_instance_plugin_base import MultipleInstancePluginBase

import cap.cap_manager
import term.parse_termdata
import term.read_termdata

from term.term_cap_handler import TermCapHandler
from term.term_buffer_handler import TermBufferHandler
from term.term_data_handler_base import TermDataHandlerBase

LOGGER = logging.getLogger('TermDataHandler')


class TempContext(object):
    pass

class DefaultTermDataHandler(MultipleInstancePluginBase,
                             TermDataHandler,
                             TermCapHandler,
                             TermBufferHandler,
                             TermDataHandlerBase):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self, name="py_term_data_handler",
                                            desc="It is a python version term data handler",
                                            version=1)
        TermDataHandler.__init__(self)
        TermCapHandler.__init__(self)
        TermBufferHandler.__init__(self)
        TermDataHandlerBase.__init__(self)

        self._stopped = True
        self._data_event = Event()
        self._data_queue = deque()
        self._process_thread = None
        self._tmp_context = TempContext()

    def init_plugin(self, context, plugin_config):
        MultipleInstancePluginBase.init_plugin(self, context, plugin_config)

        #initialize term caps
        term_name = self.plugin_context.get_app_config().get_entry("/term/term_name", "xterm-256color")
        termcap_dir = self.plugin_context.get_app_config().get_entry('/term/termcap_dir', 'data')

        self.init_with_term_name(termcap_dir, term_name)

        self.in_status_line = False
        self.keypad_transmit_mode = False
        self._has_cell_updated = False

    def on_data(self, data, count):
        self._refresh_timer.cancel()
        self._data_queue.extend(data[:count])
        self._data_event.set()

    def __on_control_data(self, cap_turple, params):
        cap_name, increase_params = cap_turple
        cap_handler = cap.cap_manager.get_cap_handler(cap_name)

        LOGGER.debug("control data:[[[" + ''.join(self._control_data) + ']]],for cap:' + cap_name)
        if not cap_handler:
            LOGGER.error('no matched:{}, params={}'.format(cap_turple, self._parse_context.params))
        elif cap_handler:
            self._tmp_context.params = params
            cap_handler.handle(self, self._tmp_context, cap_turple)

        if self._has_cell_updated:
            self.refresh_display()
            self._has_cell_updated = False

    def __output_data(self, c):
        if self.in_status_line:
            self._output_status_line_data(c)
        else:
            self._output_normal_data(c)

        self._has_cell_updated = True

    def _output_status_line_data(self, c):
        pass

    def _output_normal_data(self, c, insert=False):
        if c == '\x1b':
            LOGGER.error('normal data has escape char:{}'.format(c.replace('\x1b','\\E')))
            return

        try:
            for cc in c:
                self._save_buffer(cc, insert)
        except:
            LOGGER.exception('save buffer failed')

    def __try_parse__(self):
        try:
            while True:
                data = self._data_queue.popleft()

                if isinstance(data, str):
                    c = data[0]
                else:
                    c = chr(data)

                cap_turple, params, output_char = self.process_data(c)

                if cap_turple:
                    self.__on_control_data(cap_turple, params)

                if output_char:
                    self.__output_data(output_char)

        except IndexError:
            cap_turple, params, output_char = self.process_data()
            if cap_turple:
                self.__on_control_data(cap_turple, params)

            if output_char:
                self.__output_data(output_char)

    def __read_queued_data(self):
        while True:
            if self._stopped:
                break

            self._data_event.wait()

            if self._stopped:
                break

            if not self._data_event.is_set():
                continue

            self._data_event.clear()

            try:
                self.__try_parse__()
            except:
                LOGGER.exception("try parse failed")
                pass

            if self._stopped:
                break
            self.refresh_display(0.02)

    def start(self):
        if not self._stopped:
            return

        self._stopped = False
        self._process_thread = Thread(target=self.__read_queued_data)
        self._process_thread.start()

    def stop(self):
        if self._stopped:
            return

        self._stopped = True

        self._data_event.set()

        if (self._process_thread and
                threading.current_thread() != self._process_thread):
            self._process_thread.join()


def register_plugins(pm):
    pm.register_plugin(DefaultTermDataHandler().new_instance())
