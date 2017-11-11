#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"
/*
    def exit_alt_charset_mode(self, context):
        self.charset_modes_translate[0] = None
        self.exit_standout_mode(context)
        if self.is_debug():
            LOGGER.debug('exit alt:{} {}'.format(' at ', self.get_cursor()))

    def enter_alt_charset_mode(self, context):
        self.charset_modes_translate[0] = translate_char
        if self.is_debug():
            LOGGER.debug('enter alt:{} {}'.format(' at ', self.get_cursor()))

    def enter_alt_charset_mode_british(self, context):
        self.charset_modes_translate[0] = translate_char_british

    def enter_alt_charset_mode_g1(self, context):
        self.charset_modes_translate[1] = translate_char

    def enter_alt_charset_mode_g1_british(self, context):
        self.charset_modes_translate[1] = translate_char_british

    def exit_alt_charset_mode_g1_british(self, context):
        self.charset_modes_translate[1] = None
        self.exit_standout_mode(context)

    def shift_in_to_charset_mode_g0(self, context):
        self.charset_mode = 0

    def shift_out_to_charset_mode_g1(self, context):
        self.charset_mode = 1

    def enable_mode(self, context):
        if self.is_debug():
            LOGGER.debug('enable mode:{}'.format(context.params))

        mode = context.params[0]

        if mode == 25:
            self.cursor_normal(context)
        elif mode == 40:
            self._dec_mode = True
            self._force_column = True
            self.resize_terminal()
        elif mode == 3:
            if self._dec_mode:
                self._force_column = True
                self._force_column_count = 132
                self.resize_terminal()

                self.clr_eos(None)
                self.cursor_home(None)
        elif mode == 5:
            self.cur_cell.add_mode(TermCell.TextMode.Reverse)
        elif mode == 6:
            self._origin_mode = True
            self.cursor_home(None)
        elif mode == 7:
            self._auto_wrap = True
        else:
            LOGGER.warning('not implemented enable mode:{}'.format(context.params))

    def disable_mode(self, context):
        if self.is_debug():
            LOGGER.debug('disable mode:{}'.format(context.params))

        mode = context.params[0]

        if mode == 25:
            self.cursor_invisible(context)
        elif mode == 40:
            self._dec_mode = False
            self._force_column = False
            self.resize_terminal()
        elif mode == 3:
            if self._dec_mode:
                self._force_column = True
                self._force_column_count = 80
                self.resize_terminal()

                self.clr_eos(None)
                self.cursor_home(None)
        elif mode == 5:
            self.cur_cell.remove_mode(TermCell.TextMode.Reverse)
        elif mode == 6:
            self._origin_mode = False
            self.cursor_home(None)
        elif mode == 7:
            self._auto_wrap = False
        else:
            LOGGER.warning('not implemented disable mode:{}'.format(context.params))
    def enter_bold_mode(self, context):
        self.cur_cell.add_mode(TermCell.TextMode.Bold)
        if self.is_debug():
            LOGGER.debug('set bold mode:attr={}'.format(self.cur_cell))

    def keypad_xmit(self, context):
        if self.is_debug():
            LOGGER.debug('keypad transmit mode')
        self.keypad_transmit_mode = True
def enter_reverse_mode(self, context):
        self.cur_cell.add_mode(TermCell.TextMode.Reverse)
        self.refresh_display()

    def exit_standout_mode(self, context):
        self.cur_cell.mode = self._default_cell.mode

    def enter_ca_mode(self, context):
        self.savedcur_cell, self.cur_cell = \
            self.cur_cell, self.create_default_cell()

        self.plugin_context.term_buffer.enable_alter_buffer(True)

        self.refresh_display()

    def exit_ca_mode(self, context):
        self.cur_cell = self.savedcur_cell

        self.plugin_context.term_buffer.enable_alter_buffer(False)

        self.refresh_display()
*/
