import os
import logging
import threading
import struct

from wxglterm_interface import TermNetwork
from multiple_instance_plugin_base import MultipleInstancePluginBase

LOGGER = logging.getLogger('term_network')


class FileTermNetwork(MultipleInstancePluginBase, TermNetwork):
    def __init__(self):
        MultipleInstancePluginBase.__init__(self, name="term_network_use_file",
                                            desc="It is a python version term file_term_network",
                                            version=1)
        TermNetwork.__init__(self)

    def disconnect(self):
        pass

    def connect(self, host, port, user_name, password):
        self._file_path = file_path = self.plugin_config.get_entry("/file", "NOT FOUND")

        if not os.path.exists(file_path):
            LOGGER.error("term data file is not exist:{}".format(file_path))

        self.start_reader()

    def start_reader(self):
        term_data_handler = self.plugin_context.get_term_data_handler()

        def __read_term_data():
            with open(self._file_path, 'rb') as f:
                while True:
                    data = f.read(4)
                    if not data or len(data) != 4:
                        LOGGER.info("end of dump data, quit")
                        break
                    data_len = struct.unpack('!i', data)[0]
                    data = f.read(data_len)
                    if not data or data_len != len(data):
                        term_data_handler.on_data(data)
                        LOGGER.info("end of dump data, quit")
                        break
                    term_data_handler.on_data(data)
                return

        def read_term_data():
            try:
                __read_term_data()
            except:
                LOGGER.exception('read term data failed')

        self.reader_thread = reader_thread = threading.Thread(target=read_term_data)
        reader_thread.start()


def register_plugins(pm):
    pm.register_plugin(FileTermNetwork().new_instance())
