#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"
/*
def clear_tab(self, context):
        action = 0
        if context.params and len(context.params) > 0:
            action = context.params[0]

        if action == 0:
            self._tab_stops.pop(self.col, 0)
        elif action == 3:
            self._tab_stops.clear()

    def clear_all_tabs(self, context):
        self._tab_stops.clear()

    def set_tab(self, context):
        self._tab_stops[self.col] = True

    def tab(self, context):
        col = self.col

        if len(self._tab_stops) > 0:
            for c in range(self.col+1, TAB_MAX):
                if c in self._tab_stops:
                    col = c
                    break

        if col >= self.get_cols():
            col = self.get_cols() - 1

        self.col = col
        self.refresh_display()
*/
