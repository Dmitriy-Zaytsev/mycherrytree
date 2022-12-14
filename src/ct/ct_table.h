/*
 * ct_table.h
 *
 * Copyright 2009-2022
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#pragma once

#include "ct_codebox.h"
#include "ct_widgets.h"
#include <optional>

class CtTableCommon : public CtAnchoredWidget
{
public:
    CtTableCommon(CtMainWin* pCtMainWin,
                  const int colWidthDefault,
                  const int charOffset,
                  const std::string& justification,
                  const CtTableColWidths& colWidths,
                  const size_t currRow,
                  const size_t currCol);

    void apply_width_height(const int /*parentTextWidth*/) override {}

    const CtTableColWidths& get_col_widths_raw() const { return _colWidths; }
    int get_col_width_default() const { return _colWidthDefault; }
    int get_col_width(const std::optional<size_t> optColIdx = std::nullopt) const {
        const size_t colIdx = optColIdx.value_or(_currentColumn);
        return _colWidths.at(colIdx) != 0 ? _colWidths.at(colIdx) : _colWidthDefault;
    }
    CtTableColWidths get_col_widths() const {
        CtTableColWidths colWidths;
        for (size_t colIdx = 0; colIdx < _colWidths.size(); ++colIdx) {
            colWidths.push_back(get_col_width(colIdx));
        }
        return colWidths;
    }
    void to_xml(xmlpp::Element* p_node_parent, const int offset_adjustment, CtStorageCache* cache) override;
    bool to_sqlite(sqlite3* pDb, const gint64 node_id, const int offset_adjustment, CtStorageCache* cache) override;

    virtual void write_strings_matrix(std::vector<std::vector<Glib::ustring>>& rows) const = 0;
    virtual size_t get_num_rows() const = 0;
    virtual size_t get_num_columns() const = 0;
    size_t current_row() const { return _currentRow < get_num_rows() ? _currentRow : 0; }
    size_t current_column() const { return _currentColumn < get_num_columns() ? _currentColumn : 0; }

protected:
    virtual void _populate_xml_rows_cells(xmlpp::Element* p_table_node) const = 0;

    int              _colWidthDefault;
    CtTableColWidths _colWidths;
    size_t           _currentRow{0};
    size_t           _currentColumn{0};
};

struct CtTableLightColumns : public Gtk::TreeModelColumnRecord
{
    CtTableLightColumns(const size_t numColumns) {
        columnsText.resize(numColumns);
        for (size_t i = 0u; i < numColumns; ++i) {
            add(columnsText.at(i));
        }
        add(columnWeight);
    }
    std::vector<Gtk::TreeModelColumn<Glib::ustring>> columnsText;
    Gtk::TreeModelColumn<int>                        columnWeight;
};

class CtTableLight : public CtTableCommon
{
public:
    CtTableLight(CtMainWin* pCtMainWin,
                 CtTableMatrix& tableMatrix,
                 const int colWidthDefault,
                 const int charOffset,
                 const std::string& justification,
                 const CtTableColWidths& colWidths,
                 const size_t currRow = 0,
                 const size_t currCol = 0);

    const CtTableLightColumns& get_columns() const { return *_pColumns; }

    void apply_syntax_highlighting(const bool /*forceReApply*/) override {}
    void set_modified_false() override {}
    CtAnchWidgType get_type() override { return CtAnchWidgType::TableLight; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    void write_strings_matrix(std::vector<std::vector<Glib::ustring>>& rows) const override;
    size_t get_num_rows() const override { return _pListStore->children().size(); }
    size_t get_num_columns() const override { return _pColumns->columnsText.size(); }

protected:
    static void _free_matrix(CtTableMatrix& tableMatrix);
    void _populate_xml_rows_cells(xmlpp::Element* p_table_node) const override;

    std::unique_ptr<CtTableLightColumns> _pColumns;
    Gtk::TreeView* _pManagedTreeView;
    Glib::RefPtr<Gtk::ListStore> _pListStore;
};

class CtTable : public CtTableCommon
{
public:
    CtTable(CtMainWin* pCtMainWin,
            CtTableMatrix& tableMatrix,
            const int colWidthDefault,
            const int charOffset,
            const std::string& justification,
            const CtTableColWidths& colWidths,
            const size_t currRow = 0,
            const size_t currCol = 0);
    ~CtTable() override;

    /**
     * @brief Build a table from csv
     * The input csv should be compatable with the excel csv format
     * @param input
     * @return CtTable
     */
    static std::unique_ptr<CtTable> from_csv(const std::string& filepath,
                                             CtMainWin* main_win,
                                             const int offset,
                                             const Glib::ustring& justification);

    void apply_syntax_highlighting(const bool forceReApply) override;
    /**
     * @brief Serialise to csv format
     * The output CSV excel csv with double quotes around cells and newlines for each record
     * @param output
     */
    std::string to_csv() const;
    void set_modified_false() override;
    CtAnchWidgType get_type() override { return CtAnchWidgType::Table; }
    std::shared_ptr<CtAnchoredWidgetState> get_state() override;

    const CtTableMatrix& get_table_matrix() const { return _tableMatrix; }

    void write_strings_matrix(std::vector<std::vector<Glib::ustring>>& rows) const override;
    size_t get_num_rows() const override { return _tableMatrix.size(); }
    size_t get_num_columns() const override { return _tableMatrix.front().size(); }

    void column_add(const size_t afterColIdx);
    void column_delete(const size_t colIdx);
    void column_move_left(const size_t colIdx);
    void column_move_right(const size_t colIdx);
    void row_add(const size_t afterRowIdx, const std::vector<Glib::ustring>* pNewRow = nullptr);
    void row_delete(const size_t rowIdx);
    void row_move_up(const size_t rowIdx);
    void row_move_down(const size_t rowIdx);
    bool row_sort_asc() { return _row_sort(true/*sortAsc*/); }
    bool row_sort_desc() { return _row_sort(false/*sortAsc*/); }

    void set_col_width_default(const int colWidthDefault);
    void set_col_width(const int colWidth, std::optional<size_t> optColIdx = std::nullopt);

private:
    void _apply_styles_to_cells(const bool forceReApply);
    void _new_text_cell_attach(const size_t rowIdx, const size_t colIdx, CtTextCell* pTextCell);
    bool _row_sort(const bool sortAsc);
    void _apply_remove_header_style(const bool isApply, CtTextView& textView);

protected:
    void _populate_xml_rows_cells(xmlpp::Element* p_table_node) const override;

private:
    void _on_populate_popup_cell(Gtk::Menu* menu);
    bool _on_key_press_event_cell(GdkEventKey* event);
    bool _on_button_press_event_grid(GdkEventButton* event);
    void _on_set_focus_child_grid(Gtk::Widget* pWidget);

protected:
    CtTableMatrix    _tableMatrix;
    Gtk::Grid        _grid;
};
