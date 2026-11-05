#include <game/Editable.hpp>
#include <memory>

namespace game {
LambdaEditor::LambdaEditor(std::move_only_function<bool(void)> f)
    : m_func(std::move(f))
{
}
LambdaEditor::LambdaEditor(LambdaEditor&& o)
    : Editor::Editor(std::move(o))
    , m_func { std::move(o.m_func) }
{
    o.m_func = nullptr;
}
LambdaEditor& LambdaEditor::operator=(LambdaEditor&& o)
{
    if (&o == this)
        return *this;
    Editor::operator=(std::move(o));
    m_func = std::move(o.m_func);
    o.m_func = nullptr;
    return *this;
}
bool LambdaEditor::draw_update()
{
    return m_func();
}
std::unique_ptr<LambdaEditor> LambdaEditor::make(std::move_only_function<bool(void)> fn)
{
    return std::make_unique<LambdaEditor>(std::move(fn));
}
}
