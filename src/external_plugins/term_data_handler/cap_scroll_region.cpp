#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"

/*
def change_scroll_region(self, context):
        if self.is_debug():
            LOGGER.debug('change scroll region:{} rows={}'.format(context.params, self.get_rows()))
        if len(context.params) == 0:
            self.set_scroll_region(0, self.get_rows() - 1)
        else:
            self.set_scroll_region(context.params[0], context.params[1])
        self.cursor_home(None)

    def change_scroll_region_from_start(self, context):
        if self.is_debug():
            LOGGER.debug('change scroll region from start:{} rows={}'.format(context.params, self.get_rows()))
        self.set_scroll_region(0, context.params[0])
        self.cursor_home(None)

    def change_scroll_region_to_end(self, context):
        if self.is_debug():
            LOGGER.debug('change scroll region to end:{} rows={}'.format(context.params, self.get_rows()))
        self.set_scroll_region(context.params[0], self.get_rows() - 1)
        self.cursor_home(None)
*/
