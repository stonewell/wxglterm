#include <GLFW/glfw3.h>

static
const char * KEY_DATA[] = {
    0,
    [GLFW_KEY_ESCAPE] = "\x1b",
    [GLFW_KEY_ENTER] = "\x0d",
    [GLFW_KEY_TAB] = "\x09",
    [GLFW_KEY_BACKSPACE] = "\x7f",
    [GLFW_KEY_PAGE_UP] = "\x1B[5~",
    [GLFW_KEY_PAGE_DOWN] = "\x1B[6~",
    [GLFW_KEY_INSERT] = "\x1B[2~",
    [GLFW_KEY_DELETE] = "\x1B[3~",
    [GLFW_KEY_UP] = "\x1BOA",
    [GLFW_KEY_DOWN] = "\x1BOB",
    [GLFW_KEY_LEFT] = "\x1BOD",
    [GLFW_KEY_RIGHT] = "\x1BOC",
    [GLFW_KEY_HOME] = "\x1BOH",
    [GLFW_KEY_END] = "\x1BOF",
    [GLFW_KEY_F1] = "\x1BOP",
    [GLFW_KEY_F2] = "\x1BOQ",
    [GLFW_KEY_F3] = "\x1BOR",
    [GLFW_KEY_F4] = "\x1BOS",
    [GLFW_KEY_F5] = "\x1B[15~",
    [GLFW_KEY_F6] = "\x1B[17~",
    [GLFW_KEY_F7] = "\x1B[18~",
    [GLFW_KEY_F8] = "\x1B[19~",
    [GLFW_KEY_F9] = "\x1B[20~",
    [GLFW_KEY_F10] = "\x1B[21~",
    [GLFW_KEY_F11] = "\x1B[23~",
    [GLFW_KEY_F12] = "\x1B[24~",
    0,
    [GLFW_KEY_LAST] = 0,
};

const char * get_mapped_key_code(int key) {
    if ((size_t)key >= sizeof(KEY_DATA) / sizeof(char))
        return 0;

    return KEY_DATA[key];
}
