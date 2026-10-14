#pragma once

#include "game/XmlMarshalling.hpp"
#include <memory>
#include <optional>
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <variant>
namespace game {
struct Stylable {
    virtual ~Stylable();
    virtual void set_pos(const ::Vector2&) = 0;
    virtual void set_size(const ::Vector2&) = 0;
};

struct FontData {
    float size { };
    int spacing { };
};
struct RelativePos {
    std::string rel_to;
};

struct AsStylingParameter {
    virtual ~AsStylingParameter();
    virtual std::string to_value_string() = 0;
};

using StylingParameter = std::variant<
    int,
    float,
    double,
    std::shared_ptr<AsStylingParameter>,
    std::string>;

class StyleBuilder;
class Style {
public:
    using shared_ptr_t = std::shared_ptr<Style>;

private:
    friend class StyleBuilder;
    using custom_parameters_t = std::unordered_map<std::string, StylingParameter>;
    custom_parameters_t custom_parameters { };
    std::string m_font_name;
    ::Font m_font;
    FontData m_fontd;
    std::vector<std::string> dependencies { };
    ::Vector2 m_position { };
    std::optional<RelativePos> m_rel_pos;
    ::Vector2 m_size { };
    Style();

public:
    struct xml_node_rep {
        struct font_node {
            struct attrs {
                float size;
                std::optional<int> spacing;
            };
            static Result<std::runtime_error, font_node>
            parse_xml(pugi::xml_text txt);

            std::string font_name;
            xml::Attribute<attrs> attrs;
        } font;
        struct custom_node {
            custom_parameters_t params { };
            auto unmarshall_self(const pugi::xml_node& n)
                -> Result<std::runtime_error, Unit>;
        } custom;
        struct abs_pos_node {
            struct attrs {
                float x { }, y { };
            };
            xml::Attribute<attrs> attrs;
        };
        struct size_node {
            struct attrs {
                float width { }, height { };
            };
            xml::Attribute<attrs> attrs;
        };
    };

public:
    static Result<std::runtime_error, Style> from_xml(pugi::xml_node&);
    Style(Style&) = delete;
    Style& operator=(Style&) = delete;
    Style(Style&&);
    Style& operator=(Style&&);
    ~Style();

    const ::Font& get_font() const;
    const FontData& get_font_data() const;
    const ::Vector2& get_position() const;
    const ::Vector2& get_size() const;

    std::optional<const StylingParameter&> get_styling_param(const std::string&) const;
    template <class T>
    inline std::optional<const T*>
    get_styling_param_as(const std::string& name) const
    {
        if (!custom_parameters.contains(name))
            return std::nullopt;
        const auto& param = custom_parameters.at(name);
        if (std::holds_alternative<std::shared_ptr<AsStylingParameter>>(param)) {
            return dynamic_cast<T*>(
                std::get<std::shared_ptr<AsStylingParameter>>(param)
                    .get());
        }
        if (std::holds_alternative<T>(param)) {
            return std::get_if<T>(&param);
        }

        return std::nullopt;
    }

    // used to apply the styling on a Stylable object
    auto style(Stylable*) -> void;
    auto get_dependencies() -> const std::vector<std::string>&;

    const char*
    marshall_to_xml_name() const noexcept;
    void
    marshall_to_xml(pugi::xml_node&) const noexcept;
};

class StyleBuilder {
private:
    Style m_style;

public:
    StyleBuilder();
    StyleBuilder(StyleBuilder&) = delete;
    StyleBuilder& operator=(StyleBuilder&) = delete;
    StyleBuilder(StyleBuilder&&);
    StyleBuilder& operator=(StyleBuilder&&);
    ~StyleBuilder();

    StyleBuilder& with_font(::Font);
    StyleBuilder& with_font_data(FontData);
    StyleBuilder& with_custom_param(const std::string&, StylingParameter&&);
    Style build_style() &&;
};

}
