#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <vector>

#include <string.h>

#include "key_code_map.h"

#include "plugin_manager.h"
#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_window.h"
#include "input.h"
#include "plugin_base.h"

#include "app_config_impl.h"

#include <locale>
#include <codecvt>
#include <functional>
#include "base64.h"

static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

void send_data(TermNetworkPtr network, const std::vector<unsigned char> & data){
    try
    {
        network->Send(data, data.size());
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error Send:"
                  << std::endl
                  << e.what()
                  << std::endl;
    }
    catch(...)
    {
        std::cerr << "!!Error Send"
                  << std::endl;
    }
}
