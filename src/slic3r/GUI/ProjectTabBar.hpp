#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include <wx/panel.h>
#include <wx/string.h>

class wxBoxSizer;
class wxWindow;

namespace Slic3r {
namespace GUI {

// Print farm: the page shown for Prepare and Preview. It holds the project tab strip and one
// Plater per open project, of which only the active one is shown.
class PlaterBook : public wxPanel
{
public:
    explicit PlaterBook(wxWindow* parent);
    // The main tab bar shows and hides this page; the Slice and Print buttons follow it, as they
    // followed the Plater when it was the page itself.
    bool Show(bool show = true) override;
};

// Print farm: the strip of open project tabs above the plater, FreeCAD style.
class ProjectTabBar : public wxPanel
{
public:
    struct Callbacks
    {
        std::function<void(size_t)> select;
        std::function<void(size_t)> close;
        std::function<void()>       add;
    };

    ProjectTabBar(wxWindow* parent, Callbacks callbacks);

    // Rebuilds the strip, if the labels or the selection changed.
    void set_tabs(const std::vector<wxString>& labels, size_t selected);

private:
    void rebuild();
    // Runs a callback once the clicked control has finished its event: the callback rebuilds the strip.
    void post(std::function<void()> fn);

    Callbacks             m_callbacks;
    std::vector<wxString> m_labels;
    size_t                m_selected{0};
    wxBoxSizer*           m_sizer{nullptr};
};

} // namespace GUI
} // namespace Slic3r
