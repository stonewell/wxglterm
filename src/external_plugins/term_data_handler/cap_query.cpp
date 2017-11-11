#include "term_buffer.h"
#include "term_window.h"
#include "cap_manager.h"

/*
def client_report_version(self, context):
        self.send_data('\033[>0;136;0c')

    def user7(self, context):
        if (context.params[0] == 6):
            col, row = self.get_cursor()

            if self._origin_mode:
                begin, end = self.get_scroll_region()
                row -= begin

            self.send_data(''.join(['\x1B[', str(row + 1), ';', str(col + 1), 'R']))
        elif context.params[0] == 5:
            self.send_data('\033[0n')

    def request_background_color(self, context):
        rbg_response = '\033]11;rgb:%04x/%04x/%04x/%04x\007' % (self.cfg.default_background_color[0], self.cfg.default_background_color[1], self.cfg.default_background_color[2], self.cfg.default_background_color[3])

        if self.is_debug():
            LOGGER.debug("response background color request:{}".format(rbg_response.replace('\033', '\\E')))
        self.send_data(rbg_response)

    def user9(self, context):
        if self.is_debug():
            LOGGER.debug('response terminal type:{} {}'.format(context.params, self._cap_.cmds['user8'].cap_value))
        self.send_data(self._cap.cmds['user8'].cap_value)

    def send_primary_device_attributes(self, context):
        self.send_data('\033[?62;c')
*/
