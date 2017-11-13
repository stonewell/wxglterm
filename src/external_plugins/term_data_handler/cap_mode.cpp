#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"

#include <iostream>

DEFINE_CAP(enable_mode);
DEFINE_CAP(disable_mode);
DEFINE_CAP(enter_bold_mode);
DEFINE_CAP(enter_reverse_mode);
DEFINE_CAP(keypad_xmit);
DEFINE_CAP(exit_standout_mode);
DEFINE_CAP(enter_ca_mode);
DEFINE_CAP(exit_ca_mode);


void enable_mode(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    auto mode = params[0];

    const std::vector<int> empty_params {};

    if (mode == 25) {
        handle_cap(term_context, "cursor_normal", params);
    }
    else if (mode == 40){
        term_context.dec_mode = true;
        term_context.force_column = true;
        resize_terminal(term_context);
    }
    else if (mode == 3) {
        if (term_context.dec_mode){
            term_context.force_column = true;
            term_context.force_column_count = 132;
            resize_terminal(term_context);

            handle_cap(term_context, "clr_eos", empty_params);
            handle_cap(term_context, "cursor_home", empty_params);
        }
    }
    else if (mode == 5) {
        term_context.cell_template->AddMode(TermCell::Reverse);
    }
    else if (mode == 6) {
        term_context.origin_mode = true;
        handle_cap(term_context, "cursor_home", empty_params);
    }
    else if (mode == 7) {
        term_context.auto_wrap = true;
    }
    else {
        std::cerr << "enable mode not implemented, with params:[";
        std::copy(params.begin(), params.end(), std::ostream_iterator<int>(std::cerr, ","));
        std::cerr << "]" << std::endl;
    }
}

void disable_mode(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    auto mode = params[0];

    const std::vector<int> empty_params {};

    if (mode == 25) {
        handle_cap(term_context, "cursor_invisible", params);
    }
    else if (mode == 40){
        term_context.dec_mode = false;
        term_context.force_column = false;
        resize_terminal(term_context);
    }
    else if (mode == 3) {
        if (term_context.dec_mode){
            term_context.force_column = true;
            term_context.force_column_count = 80;
            resize_terminal(term_context);

            handle_cap(term_context, "clr_eos", empty_params);
            handle_cap(term_context, "cursor_home", empty_params);
        }
    }
    else if (mode == 5) {
        term_context.cell_template->RemoveMode(TermCell::Reverse);
    }
    else if (mode == 6) {
        term_context.origin_mode = false;
        handle_cap(term_context, "cursor_home", empty_params);
    }
    else if (mode == 7) {
        term_context.auto_wrap = false;
    }
    else {
        std::cerr << "disable mode not implemented, with params:[";
        std::copy(params.begin(), params.end(), std::ostream_iterator<int>(std::cerr, ","));
        std::cerr << "]" << std::endl;
    }
}

void enter_bold_mode(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.cell_template->AddMode(TermCell::Bold);
    term_context.term_buffer->AddMode(TermCell::Bold);
}
void keypad_xmit(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.keypad_transmit_mode = true;
}
void enter_reverse_mode(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.cell_template->AddMode(TermCell::Reverse);
    term_context.term_buffer->AddMode(TermCell::Reverse);
}

void exit_standout_mode(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.cell_template->SetMode(term_context.default_cell_template->GetMode());
    term_context.term_buffer->SetMode(term_context.default_cell_template->GetMode());
}

void enter_ca_mode(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.saved_cell_template = term_context.cell_template;
    term_context.cell_template = term_context.term_buffer->CreateCellWithDefaults();
    term_context.term_buffer->EnableAlterBuffer(true);
}
void exit_ca_mode(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.cell_template = term_context.saved_cell_template;
    term_context.term_buffer->EnableAlterBuffer(false);
}
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
*/
