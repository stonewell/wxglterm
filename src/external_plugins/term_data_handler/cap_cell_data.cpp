#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"

DEFINE_CAP(screen_alignment_test);
DEFINE_CAP(insert_line);
DEFINE_CAP(parm_insert_line);
DEFINE_CAP(delete_line);
DEFINE_CAP(parm_delete_line);
DEFINE_CAP(clr_eos);
DEFINE_CAP(clr_line);
DEFINE_CAP(clr_eol);
DEFINE_CAP(clr_bol);
DEFINE_CAP(parm_ich);
DEFINE_CAP(parm_dch);
DEFINE_CAP(erase_chars);

void screen_alignment_test(term_data_context_s & term_context,
                           const std::vector<int> & params) {
    handle_cap(term_context, "save_cursor", params);
    auto rows = term_context.term_buffer->GetRows();
    auto cols = term_context.term_buffer->GetCols();

    for(uint32_t row = 0; row < rows; row++) {
        auto line = term_context.term_buffer->GetLine(row);

        for(uint32_t col=0; col < cols; col++) {
            auto cell = line->GetCell(col);

            if (cell)
                cell->SetChar('E');
        }
    }
    handle_cap(term_context, "restore_cursor", params);
}

void insert_line(term_data_context_s & term_context,
                 const std::vector<int> & params) {
    parm_insert_line(term_context,
                     params);
}

void parm_insert_line(term_data_context_s & term_context,
                      const std::vector<int> & params) {
    auto count = 1;

    if (params.size() > 0)
        count = params[0];

    term_context.term_buffer->InsertLines(term_context.term_buffer->GetRow(),
                                          count);
}

void delete_line(term_data_context_s & term_context,
                 const std::vector<int> & params) {
    parm_delete_line(term_context,
                     params);
}

void parm_delete_line(term_data_context_s & term_context,
                      const std::vector<int> & params) {
    auto count = 1;

    if (params.size() > 0)
        count = params[0];

    term_context.term_buffer->DeleteLines(term_context.term_buffer->GetRow(),
                                          count);
}

void clr_eos(term_data_context_s & term_context,
             const std::vector<int> & params) {
    uint32_t begin = 0, end = term_context.term_buffer->GetRows();
    uint32_t cols = term_context.term_buffer->GetCols();

    if (params.size() == 0 || params[0] == 0) {
        clr_eol(term_context, params);
        begin = term_context.term_buffer->GetRow() + 1;
    } else if (params[0] == 1) {
        clr_bol(term_context, params);
        end = term_context.term_buffer->GetRow();
    }

    for(auto row = begin; row < end; row++) {
        auto line = term_context.term_buffer->GetLine(row);

        if (!line)
            continue;

        for(uint32_t col = 0; col < cols; col++) {
            auto cell = line->GetCell(col);

            if (cell) {
                cell->Reset(term_context.cell_template);
            }
        }
    }
}

void clr_line(term_data_context_s & term_context,
              const std::vector<int> & params) {
    (void)params;
    auto line = term_context.term_buffer->GetCurLine();

    if (!line)
        return;

    uint32_t cols = term_context.term_buffer->GetCols();

    for(uint32_t col = 0; col < cols; col++) {
        auto cell = line->GetCell(col);

        if (cell) {
            cell->Reset(term_context.cell_template);
        }
    }
}

void clr_eol(term_data_context_s & term_context,
             const std::vector<int> & params) {
    (void)params;
    auto line = term_context.term_buffer->GetCurLine();

    if (!line)
        return;

    uint32_t cols = term_context.term_buffer->GetCols();
    uint32_t begin = term_context.term_buffer->GetCol();

    if (begin < cols && line->GetCell(begin)->GetChar() == '\000')
        begin--;

    for(uint32_t col = begin; col < cols; col++) {
        auto cell = line->GetCell(col);

        if (cell) {
            cell->Reset(term_context.cell_template);
        }
    }
}

void clr_bol(term_data_context_s & term_context,
             const std::vector<int> & params) {
    (void)params;
    auto line = term_context.term_buffer->GetCurLine();

    if (!line)
        return;

    uint32_t cols = term_context.term_buffer->GetCols();
    uint32_t end = term_context.term_buffer->GetCol();

    if (end + 1 < cols && line->GetCell(end + 1)->GetChar() == '\000')
        end++;

    for(uint32_t col = 0; col <= end; col++) {
        auto cell = line->GetCell(col);

        if (cell) {
            cell->Reset(term_context.cell_template);
        }
    }
}

void parm_ich(term_data_context_s & term_context,
              const std::vector<int> & params) {
    for(int i=0;i < params[0];i++)
        output_char(term_context,
                    " ",
                    true);
}

static
void __do_delete_chars(term_data_context_s & term_context,
                       int count,
                       bool overwrite) {
    auto line = term_context.term_buffer->GetCurLine();

    if (!line)
        return;

    uint32_t cols = term_context.term_buffer->GetCols();
    uint32_t begin = term_context.term_buffer->GetCol();

    if (begin < cols && line->GetCell(begin)->GetChar() == '\000')
        begin--;

    auto end = begin + count;

    if (!overwrite || (begin + count > cols))
        end = cols;

    for(auto col = begin; col < end; col++) {
        if (!overwrite && (col + count < cols)) {
            line->GetCell(col)->Reset(line->GetCell(col + count));
        } else {
            line->GetCell(col)->Reset(term_context.cell_template);
        }
    }
}

void parm_dch(term_data_context_s & term_context,
              const std::vector<int> & params) {
    __do_delete_chars(term_context,
                      params[0],
                      false);
}

void erase_chars(term_data_context_s & term_context,
                 const std::vector<int> & params) {
    __do_delete_chars(term_context,
                      params[0],
                      true);
}
