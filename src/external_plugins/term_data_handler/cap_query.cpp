#include <pybind11/embed.h>

#include "term_buffer.h"
#include "term_window.h"
#include "term_network.h"
#include "cap_manager.h"

#include <string.h>
#include <iostream>

DEFINE_CAP(client_report_version);
DEFINE_CAP(user7);
DEFINE_CAP(send_primary_device_attributes);
DEFINE_CAP(user9);

static
void send(term_data_context_s & term_context,
          const char * data) {
    std::vector<unsigned char> _data;
    std::copy(data, data + strlen(data),
              std::back_inserter(_data));

    try
    {
        term_context.term_network->Send(_data, _data.size());
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error Send:"
                  << std::endl
                  << e.what()
                  << std::endl;
        PyErr_Print();
    }
    catch(...)
    {
        std::cerr << "!!Error Send"
                  << std::endl;
        PyErr_Print();
    }
};

void client_report_version(term_data_context_s & term_context,
                           const term_data_param_list & params){
    (void)params;
    const char * client_version = "\033[>0;136;0c";

    send(term_context, client_version);
}

void user7(term_data_context_s & term_context,
           const term_data_param_list & params){
    if (params[0] == 6) {
        auto col = term_context.term_buffer->GetCol();
        auto row = term_context.term_buffer->GetRow();

        if (term_context.origin_mode) {
            row -= term_context.term_buffer->GetScrollRegionBegin();
        }

        char buf[256]{0};
        sprintf(buf, "\x1B[%d;%dR", row + 1, col + 1);

        send(term_context, buf);
    } else if (params[0] == 5) {
        const char * user7_str_5 = "\033[0n";
        send(term_context, user7_str_5);
    }
}

void send_primary_device_attributes(term_data_context_s & term_context,
                                    const term_data_param_list & params){
    (void)params;
    const char * data = "\033[?62;c";
    send(term_context, data);
}

void user9(term_data_context_s & term_context,
           const term_data_param_list & params){
    (void)params;
    const char * data = "\033[?1;2c";
    send(term_context, data);
}

/*
  def request_background_color(self, context):
  rbg_response = '\033]11;rgb:%04x/%04x/%04x/%04x\007' % (self.cfg.default_background_color[0], self.cfg.default_background_color[1], self.cfg.default_background_color[2], self.cfg.default_background_color[3])

  if self.is_debug():
  LOGGER.debug("response background color request:{}".format(rbg_response.replace('\033', '\\E')))
  self.send_data(rbg_response)
*/
