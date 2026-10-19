#pragma once
#include "components/AutoMoveD.hpp"
#include "components/GameGraphElements.hpp"
#include "components/Memory.hpp"
#include "facelift/Fl.hpp"

namespace components {

class Display : public components::OComponent {
public:
    using display_msg_t = mem::MemoryWrite;

    inline static const ::Color BODY_COLOR = ::GRAY;
    inline static const ::Color SCREEN_COLOR = ::BLACK;

private:
    struct data {
        std::optional<int> val { std::nullopt };
        std::string val_rep { };
    };
    AutoMoveD<data> m_d { };

public:
    Display(std::string name);
    Display(fl::string_param_t name);
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
    Display(Display&& o);
    Display& operator=(Display&& o);
    virtual ~Display();

    virtual void draw() override;
    virtual void update() override;
    virtual machine::actor::Actor<void> poll(machine::Mctx) override;

    // virtual void set_size(const ::Vector2&);
    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;
};

}
