#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"
#include "string_utils.h"

#include <iostream>
#include <iterator> // for ostream_iterator

DEFINE_CAP(enable_mode);
DEFINE_CAP(disable_mode);
DEFINE_CAP(enter_bold_mode);
DEFINE_CAP(enter_reverse_mode);
DEFINE_CAP(keypad_xmit);
DEFINE_CAP(exit_standout_mode);
DEFINE_CAP(enter_ca_mode);
DEFINE_CAP(exit_ca_mode);
DEFINE_CAP(exit_alt_charset_mode);
DEFINE_CAP(enter_alt_charset_mode);
DEFINE_CAP(enter_alt_charset_mode_british);
DEFINE_CAP(exit_alt_charset_mode_g1_british);
DEFINE_CAP(enter_alt_charset_mode_g1);
DEFINE_CAP(enter_alt_charset_mode_g1_british);
DEFINE_CAP(shift_in_to_charset_mode_g0);
DEFINE_CAP(shift_in_to_charset_mode_g1);

static
const
wchar_t
line_drawing_map[] = {
    0x25c6,  // ` => diamond
    0x2592,  // a => checkerboard
    0x2409,  // b => HT symbol
    0x240c,  // c => FF symbol
    0x240d,  // d => CR symbol
    0x240a,  // e => LF symbol
    0x00b0,  // f => degree
    0x00b1,  // g => plus/minus
    0x2424,  // h => NL symbol
    0x240b,  // i => VT symbol
    0x2518,  // j => downright corner
    0x2510,  // k => upright corner
    0x250c,  // l => upleft corner
    0x2514,  // m => downleft corner
    0x253c,  // n => cross
    0x23ba,  // o => scan line 1/9
    0x23bb,  // p => scan line 3/9
    0x2500,  // q => horizontal line (also scan line 5/9)
    0x23bc,  // r => scan line 7/9
    0x23bd,  // s => scan line 9/9
    0x251c,  // t => left t
    0x2524,  // u => right t
    0x2534,  // v => bottom t
    0x252c,  // w => top t
    0x2502,  // x => vertical line
    0x2264,  // y => <=
    0x2265,  // z => >=
    0x03c0,  // { => pi
    0x2260,  // | => not equal
    0x00a3,  // } => pound currency sign
    0x00b7,  // ~ => bullet
};

static
wchar_t translate_char(wchar_t c) {
    if (c >= 96 && c < 126)
        return line_drawing_map[c - 96];
    return c;
}

static
wchar_t translate_char_british(wchar_t c) {
    if (c == '#')
        return 0x00a3;

    return c;
}

void enable_mode(term_data_context_s & term_context,
                 const term_data_param_list & params) {
    auto mode = params[0];

    const term_data_param_list empty_params {};

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
    else if (mode == 1047) {
        if (term_context.cap_debug) {
            std::cerr << "enable alter buffer" << std::endl;
        }
        handle_cap(term_context, "enter_ca_mode", empty_params);
    }
    else if (mode == 1048) {
        if (term_context.cap_debug) {
            std::cerr << "save cursor" << std::endl;
        }
        handle_cap(term_context, "save_cursor", empty_params);
    }
    else if (mode == 1049) {
        if (term_context.cap_debug) {
            std::cerr << "enter alter buffer and save cursor" << std::endl;
        }
        handle_cap(term_context, "save_cursor", empty_params);
        handle_cap(term_context, "enter_ca_mode", empty_params);
    }
    else if (mode == 1002) {
        if (term_context.cap_debug) {
            std::cerr << "enable cell motion mouse tracking" << std::endl;
        }

        term_context.cell_motion_mouse_track = true;
    }
    else if (mode == 1004) {
        if (term_context.cap_debug) {
            std::cerr << "send focus in and out event" << std::endl;
        }

        term_context.send_focus_in_out = true;
    }
    else if (mode == 1006) {
        if (term_context.cap_debug) {
            std::cerr << "enable sgr mouse mode" << std::endl;
        }

        term_context.sgr_mouse_mode = true;
    }
    else {
        std::cerr << "enable mode not implemented, with params:["
                  << join(params, ",");
        std::cerr << "]" << std::endl;
    }
}

void disable_mode(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    auto mode = params[0];

    const term_data_param_list empty_params {};

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
    else if (mode == 12) {
        if (term_context.cap_debug) {
            std::cerr << "disable cursor blinking" << std::endl;
        }
    }
    else if (mode == 1047) {
        if (term_context.cap_debug) {
            std::cerr << "disable alter buffer" << std::endl;
        }
        handle_cap(term_context, "exit_ca_mode", empty_params);
    }
    else if (mode == 1048) {
        if (term_context.cap_debug) {
            std::cerr << "restore cursor" << std::endl;
        }
        handle_cap(term_context, "restore_cursor", empty_params);
    }
    else if (mode == 1049) {
        if (term_context.cap_debug) {
            std::cerr << "disable alter buffer and restore cursor" << std::endl;
        }
        handle_cap(term_context, "exit_ca_mode", empty_params);
        handle_cap(term_context, "restore_cursor", empty_params);
    }
    else if (mode == 1002) {
        if (term_context.cap_debug) {
            std::cerr << "disable cell motion mouse tracking" << std::endl;
        }

        term_context.cell_motion_mouse_track = false;
    }
    else if (mode == 1004) {
        if (term_context.cap_debug) {
            std::cerr << "disable send focus in and out event" << std::endl;
        }

        term_context.send_focus_in_out = false;
    }
    else if (mode == 1006) {
        if (term_context.cap_debug) {
            std::cerr << "disable sgr mouse mode" << std::endl;
        }

        term_context.sgr_mouse_mode = false;
    }
    else {
        std::cerr << "disable mode not implemented, with params:["
                  << join(params, ",");
        std::cerr << "]" << std::endl;
    }
}

void enter_bold_mode(term_data_context_s & term_context,
                     const term_data_param_list & params) {
    (void)params;
    term_context.cell_template->AddMode(TermCell::Bold);
}
void keypad_xmit(term_data_context_s & term_context,
                 const term_data_param_list & params) {
    (void)params;
    term_context.keypad_transmit_mode = true;
}
void enter_reverse_mode(term_data_context_s & term_context,
                        const term_data_param_list & params) {
    (void)params;
    term_context.cell_template->AddMode(TermCell::Reverse);
}

void exit_standout_mode(term_data_context_s & term_context,
                        const term_data_param_list & params) {
    (void)params;
    term_context.cell_template->SetMode(term_context.default_cell_template->GetMode());
}

void enter_ca_mode(term_data_context_s & term_context,
                   const term_data_param_list & params) {
    (void)params;
    term_context.saved_cell_template = term_context.cell_template;
    term_context.cell_template = term_context.term_buffer->CreateCellWithDefaults();
    term_context.term_buffer->EnableAlterBuffer(true);
}
void exit_ca_mode(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    (void)params;
    term_context.cell_template = term_context.saved_cell_template;
    term_context.term_buffer->EnableAlterBuffer(false);
}

void exit_alt_charset_mode(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    term_context.charset_modes_translate[0] = nullptr;
    exit_standout_mode(term_context, params);
}
void enter_alt_charset_mode(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    (void)params;
    term_context.charset_modes_translate[0] = translate_char;
}
void enter_alt_charset_mode_british(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    (void)params;
    term_context.charset_modes_translate[0] = translate_char_british;
}
void enter_alt_charset_mode_g1(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    (void)params;
    term_context.charset_modes_translate[1] = translate_char;
}
void enter_alt_charset_mode_g1_british(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    (void)params;
    term_context.charset_modes_translate[1] = translate_char_british;
}
void exit_alt_charset_mode_g1_british(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    term_context.charset_modes_translate[1] = nullptr;
    exit_standout_mode(term_context, params);
}
void shift_in_to_charset_mode_g0(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    (void)params;
    term_context.charset_mode = 0;
}
void shift_in_to_charset_mode_g1(term_data_context_s & term_context,
                  const term_data_param_list & params) {
    (void)params;
    term_context.charset_mode = 1;
}
