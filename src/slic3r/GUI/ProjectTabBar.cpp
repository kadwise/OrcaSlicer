#include "ProjectTabBar.hpp"

#include <utility>

#include <wx/colour.h>
#include <wx/cursor.h>
#include <wx/event.h>
#include <wx/font.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "GUI_App.hpp"
#include "I18N.hpp"
#include "MainFrame.hpp"
#include "Widgets/StateColor.hpp"

namespace Slic3r {
namespace GUI {

PlaterBook::PlaterBook(wxWindow* parent) : wxPanel(parent, wxID_ANY) {}

bool PlaterBook::Show(bool show)
{
    if (wxGetApp().mainframe)
        wxGetApp().mainframe->show_option(show);
    return wxPanel::Show(show);
}

ProjectTabBar::ProjectTabBar(wxWindow* parent, Callbacks callbacks)
    : wxPanel(parent, wxID_ANY), m_callbacks(std::move(callbacks))
{
    SetBackgroundColour(StateColor::darkModeColorFor(wxColour("#E6E6E8")));
    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(m_sizer);
    rebuild();
}

void ProjectTabBar::set_tabs(const std::vector<wxString>& labels, size_t selected)
{
    if (labels == m_labels && selected == m_selected)
        return;
    m_labels   = labels;
    m_selected = selected;
    rebuild();
}

void ProjectTabBar::post(std::function<void()> fn)
{
    CallAfter([fn = std::move(fn)]() {
        if (fn)
            fn();
    });
}

void ProjectTabBar::rebuild()
{
    Freeze();
    m_sizer->Clear(true);

    const wxColour bar_bg      = StateColor::darkModeColorFor(wxColour("#E6E6E8"));
    const wxColour tab_bg      = StateColor::darkModeColorFor(wxColour("#D5D5D8"));
    const wxColour selected_bg = StateColor::darkModeColorFor(wxColour("#FFFFFF"));
    const wxColour text        = StateColor::darkModeColorFor(wxColour("#262E30"));
    const wxColour accent      = wxColour("#009688");
    SetBackgroundColour(bar_bg);

    const int pad = FromDIP(6);
    for (size_t i = 0; i < m_labels.size(); ++i) {
        const bool selected = i == m_selected;
        auto*      tab      = new wxPanel(this, wxID_ANY);
        tab->SetBackgroundColour(selected ? selected_bg : tab_bg);
        tab->SetCursor(wxCursor(wxCURSOR_HAND));
        auto* tab_sizer = new wxBoxSizer(wxVERTICAL);
        auto* row       = new wxBoxSizer(wxHORIZONTAL);

        auto*  label = new wxStaticText(tab, wxID_ANY, m_labels[i]);
        wxFont font  = wxGetApp().normal_font();
        if (selected)
            font.SetWeight(wxFONTWEIGHT_BOLD);
        label->SetFont(font);
        label->SetForegroundColour(text);
        label->SetToolTip(m_labels[i]);

        auto* close = new wxStaticText(tab, wxID_ANY, wxString::FromUTF8("\xC3\x97"));
        close->SetFont(font);
        close->SetForegroundColour(text);
        close->SetToolTip(_L("Close this project"));

        row->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(10));
        row->Add(close, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(8));
        // A line under the selected tab, so it reads as the one in front.
        auto* underline = new wxPanel(tab, wxID_ANY, wxDefaultPosition, wxSize(-1, FromDIP(2)));
        underline->SetBackgroundColour(selected ? accent : tab->GetBackgroundColour());
        tab_sizer->Add(row, 1, wxEXPAND | wxTOP | wxBOTTOM, pad);
        tab_sizer->Add(underline, 0, wxEXPAND);
        tab->SetSizer(tab_sizer);

        auto on_select = [this, i](wxMouseEvent&) { post([this, i]() { if (m_callbacks.select) m_callbacks.select(i); }); };
        auto on_close  = [this, i](wxMouseEvent&) { post([this, i]() { if (m_callbacks.close) m_callbacks.close(i); }); };
        tab->Bind(wxEVT_LEFT_UP, on_select);
        label->Bind(wxEVT_LEFT_UP, on_select);
        // Middle click closes a tab, as in a web browser.
        tab->Bind(wxEVT_MIDDLE_UP, on_close);
        label->Bind(wxEVT_MIDDLE_UP, on_close);
        close->Bind(wxEVT_LEFT_UP, on_close);

        m_sizer->Add(tab, 0, wxEXPAND | wxRIGHT, FromDIP(1));
    }

    auto* add = new wxStaticText(this, wxID_ANY, "+");
    wxFont add_font = wxGetApp().bold_font();
    add->SetFont(add_font);
    add->SetForegroundColour(text);
    add->SetCursor(wxCursor(wxCURSOR_HAND));
    add->SetToolTip(_L("New project tab"));
    add->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) { post([this]() { if (m_callbacks.add) m_callbacks.add(); }); });
    m_sizer->Add(add, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(10));

    Layout();
    if (GetParent() != nullptr)
        GetParent()->Layout();
    Thaw();
}

} // namespace GUI
} // namespace Slic3r
