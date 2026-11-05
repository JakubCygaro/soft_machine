#pragma once

#include <functional>
#include <memory>
namespace game {
struct Editor {
    virtual ~Editor() = default;
    Editor() = default;
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
    Editor(Editor&&) = default;
    Editor& operator=(Editor&&) = default;

    virtual bool draw_update() = 0;
};

class LambdaEditor : public Editor {
    std::move_only_function<bool(void)> m_func;

public:
    LambdaEditor() = delete;
    LambdaEditor(std::move_only_function<bool(void)>);
    LambdaEditor(LambdaEditor&&);
    LambdaEditor& operator=(LambdaEditor&&);
    virtual bool draw_update() override;
    static std::unique_ptr<LambdaEditor> make(std::move_only_function<bool(void)>);
};

struct Editable {
    inline virtual ~Editable() { }
    /// the return value indicates whether a change to the underlying object occured
    // virtual bool draw_edit_window() = 0;
    virtual std::unique_ptr<Editor> get_editor_object() = 0;
    /// handler for when the object is notified that something has changed
    inline virtual void on_notified() { };
};
}
