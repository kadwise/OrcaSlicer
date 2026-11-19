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
    SetBackgroundColour(StateColor::darkModeColorFor(wxColour("#E9E4D8")));
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

    // KadWise, FreeCAD style: a cream strip under the 3D view; the selected tab is lighter,
    // with a yellow line on top and navy text.
    const wxColour bar_bg      = StateColor::darkModeColorFor(wxColour("#E9E4D8"));
    const wxColour selected_bg = StateColor::darkModeColorFor(wxColour("#F7F1E3"));
    const wxColour text        = StateColor::darkModeColorFor(wxColour("#4A5263"));
    const wxColour selected_fg = StateColor::darkModeColorFor(wxColour("#1F3A68"));
    const wxColour accent      = wxColour("#F2B705");
    SetBackgroundColour(bar_bg);

    const int pad = FromDIP(6);
    m_sizer->AddSpacer(FromDIP(8));
    for (size_t i = 0; i < m_labels.size(); ++i) {
        const bool selected = i == m_selected;
        auto*      tab      = new wxPanel(this, wxID_ANY);
        tab->SetBackgroundColour(selected ? selected_bg : bar_bg);
        tab->SetCursor(wxCursor(wxCURSOR_HAND));
        auto* tab_sizer = new wxBoxSizer(wxVERTICAL);
        auto* row       = new wxBoxSizer(wxHORIZONTAL);

        auto*  label = new wxStaticText(tab, wxID_ANY, m_labels[i]);
        wxFont font  = wxGetApp().normal_font();
        if (selected)
            font.SetWeight(wxFONTWEIGHT_BOLD);
        label->SetFont(font);
        label->SetForegroundColour(selected ? selected_fg : text);
        label->SetToolTip(m_labels[i]);

        auto* close = new wxStaticText(tab, wxID_ANY, wxString::FromUTF8("\xC3\x97"));
        close->SetFont(wxGetApp().normal_font());
        close->SetForegroundColour(text);
        close->SetToolTip(_L("Close this project"));

        row->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(12));
        row->Add(close, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(8));
        // The line on top of the selected tab, so it reads as the one in front.
        auto* marker = new wxPanel(tab, wxID_ANY, wxDefaultPosition, wxSize(-1, FromDIP(3)));
        marker->SetBackgroundColour(selected ? accent : tab->GetBackgroundColour());
        tab_sizer->Add(marker, 0, wxEXPAND);
        tab_sizer->Add(row, 1, wxEXPAND | wxTOP | wxBOTTOM, pad);
        tab->SetSizer(tab_sizer);

        auto on_select = [this, i](wxMouseEvent&) { post([this, i]() { if (m_callbacks.select) m_callbacks.select(i); }); };
        auto on_close  = [this, i](wxMouseEvent&) { post([this, i]() { if (m_callbacks.close) m_callbacks.close(i); }); };
        tab->Bind(wxEVT_LEFT_UP, on_select);
        label->Bind(wxEVT_LEFT_UP, on_select);
        // Middle click closes a tab, as in a web browser.
        tab->Bind(wxEVT_MIDDLE_UP, on_close);
        label->Bind(wxEVT_MIDDLE_UP, on_close);
        close->Bind(wxEVT_LEFT_UP, on_close);

        m_sizer->Add(tab, 0, wxEXPAND | wxRIGHT, FromDIP(2));
    }

    auto* add = new wxStaticText(this, wxID_ANY, "+");
    add->SetFont(wxGetApp().bold_font());
    add->SetForegroundColour(selected_fg);
    add->SetCursor(wxCursor(wxCURSOR_HAND));
    add->SetToolTip(_L("New project tab"));
    add->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) { post([this]() { if (m_callbacks.add) m_callbacks.add(); }); });
    m_sizer->Add(add, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(12));

    Layout();
    if (GetParent() != nullptr)
        GetParent()->Layout();
    Thaw();
}

} // namespace GUI
} // namespace Slic3r
