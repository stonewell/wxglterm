import array
import fcntl
import os
import select
import termios
import logging

import client.pty_client
from .session import Session

LOGGER = logging.getLogger('session')


class PtySession(Session):
    def __init__(self, cfg, terminal):
        super(PtySession, self).__init__(cfg, terminal)

        self.channel = None

    def _read_data(self, block_size = 8192):
        if not self.channel:
            return None

        if self.stopped:
            return None

        while True:
            rlist, wlist, elist = select.select([self.channel], [], [self.channel], 1)

            if self.stopped or len(elist) > 0:
                return None

            if len(rlist) > 0:
                break

        data = []

        while True:
            try:
                tmp_data = os.read(self.channel, block_size)

                if self.stopped:
                    return None

                if len(tmp_data) == 0:
                    break
                data += tmp_data
            except:
                break

        return data

    def _stop_reader(self):
        if self.channel:
            os.close(self.channel)
            self.channel = None

    def interactive_shell(self, channel):
        self.channel = channel
        self.oldflags = fcntl.fcntl(self.channel, fcntl.F_GETFL)
	    # make the PTY non-blocking
        fcntl.fcntl(self.channel, fcntl.F_SETFL, self.oldflags | os.O_NONBLOCK)

        self.resize_pty()

        self._start_reader()

    def start(self):
        super(PtySession, self).start()

        client.pty_client.start_client(self, self.cfg)

    def send(self, data):
        if self.channel and not self.stopped:
            while len(data) != 0:
                select.select([], [self.channel], [self.channel])
                n = os.write(self.channel, data)
                data = data[n:]

    def resize_pty(self, col = None, row = None, w = 0, h = 0):
        if not col:
            col = self.terminal.get_cols()
        if not row:
            row = self.terminal.get_rows()

        if self.channel and not self.stopped:
            LOGGER.error('resize pty row:{}, col:{}, w:{}, h:{}'.format(row, col, w, h))
            buf = array.array('h', [row, col, int(h), int(w)])
            fcntl.ioctl(self.channel, termios.TIOCSWINSZ, buf)

    def on_status_line(self, mode, status_line):
        pass
