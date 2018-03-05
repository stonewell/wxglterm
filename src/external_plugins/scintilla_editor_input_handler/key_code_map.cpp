#include "input.h"
#include "key_code_map.h"

static
const char * KEY_DATA[InputHandler::KEY_LAST + 1] = {0};

static
void init_key_data()  {
    KEY_DATA[InputHandler::KEY_ESCAPE] = "\x1b";
    KEY_DATA[InputHandler::KEY_TAB] = "\x09";
    KEY_DATA[InputHandler::KEY_BACKSPACE] = "\x7f";
    KEY_DATA[InputHandler::KEY_PAGE_UP] = "\x1B[5~";
    KEY_DATA[InputHandler::KEY_PAGE_DOWN] = "\x1B[6~";
    KEY_DATA[InputHandler::KEY_INSERT] = "\x1B[2~";
    KEY_DATA[InputHandler::KEY_DELETE] = "\x1B[3~";
    KEY_DATA[InputHandler::KEY_UP] = "\x1B[1A";
    KEY_DATA[InputHandler::KEY_DOWN] = "\x1B[1B";
    KEY_DATA[InputHandler::KEY_LEFT] = "\x1B[1D";
    KEY_DATA[InputHandler::KEY_RIGHT] = "\x1B[1C";
    KEY_DATA[InputHandler::KEY_HOME] = "\x0D";
    KEY_DATA[InputHandler::KEY_END] = "\x1BOF";
    KEY_DATA[InputHandler::KEY_F1] = "\x1BOP";
    KEY_DATA[InputHandler::KEY_F2] = "\x1BOQ";
    KEY_DATA[InputHandler::KEY_F3] = "\x1BOR";
    KEY_DATA[InputHandler::KEY_F4] = "\x1BOS";
    KEY_DATA[InputHandler::KEY_F5] = "\x1B[15~";
    KEY_DATA[InputHandler::KEY_F6] = "\x1B[17~";
    KEY_DATA[InputHandler::KEY_F7] = "\x1B[18~";
    KEY_DATA[InputHandler::KEY_F8] = "\x1B[19~";
    KEY_DATA[InputHandler::KEY_F9] = "\x1B[20~";
    KEY_DATA[InputHandler::KEY_F10] = "\x1B[21~";
    KEY_DATA[InputHandler::KEY_F11] = "\x1B[23~";
    KEY_DATA[InputHandler::KEY_F12] = "\x1B[24~";
    KEY_DATA[InputHandler::KEY_ENTER] = "\x1B[OM";
    KEY_DATA[InputHandler::KEY_LAST] = 0;
};

const char * get_mapped_key_code(int key) {
    static bool init = false;

    if (!init) {
        init = true;
        init_key_data();
    }

    if ((size_t)key >= sizeof(KEY_DATA) / sizeof(char))
        return 0;

    return KEY_DATA[key];
}
