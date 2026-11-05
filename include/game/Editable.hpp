#pragma once

namespace game {
struct Editable {
    inline virtual ~Editable() { }
    /// the return value indicates whether a change to the underlying object occured
    virtual bool draw_edit_window() = 0;
    /// handler for when the object is notified that something has changed
    inline virtual void on_notified() { };
};
}
