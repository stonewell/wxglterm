#include <iostream>
#include <cstdint>
#include <cassert>

#include "utf8_util.h"

static
size_t sequence_length(char * lead_it) {
    uint8_t lead = (*lead_it) & 0xFF;
    if (lead < 0x80)
        return 1;
    else if ((lead >> 5) == 0x6)
        return 2;
    else if ((lead >> 4) == 0xe)
        return 3;
    else if ((lead >> 3) == 0x1e)
        return 4;
    else {
        assert(false);
        return 0;
    }
}

size_t utf8_to_wchar(char * in, size_t in_size, wchar_t & codepoint) {
    auto seq_len = sequence_length(in);

    if (seq_len > in_size)
        return 0;

    // assert(seq_len == in_size);
    // assert(sizeof(wchar_t) > 2);

    size_t index = 0;

    while(index < seq_len) {
        unsigned char ch = static_cast<unsigned char>(in[index]);

        if (ch <= 0x7f) {
            // assert(seq_len == 1);
            codepoint = ch;
        }
        else if (ch <= 0xbf)
            codepoint = (codepoint << 6) | (ch & 0x3f);
        else if (ch <= 0xdf)
            codepoint = ch & 0x1f;
        else if (ch <= 0xef)
            codepoint = ch & 0x0f;
        else
            codepoint = ch & 0x07;

        index++;
    }

    return seq_len;
}
