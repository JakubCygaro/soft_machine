#pragma once

#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include <memory>
#include <optional>
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <variant>
namespace game {
struct FontData {
    float size { };
    int spacing { };
};

struct AsStylingParameter {
    virtual ~AsStylingParameter();
};

using StylingParameter = std::variant<
    int,
    float,
    double,
    std::shared_ptr<AsStylingParameter>,
    std::string>;

class StyleBuilder;
class Style {
private:
    friend class StyleBuilder;
    using custom_parameters_t = std::unordered_map<std::string, StylingParameter>;
    custom_parameters_t custom_parameters { };
    ::Font m_font;
    FontData m_fontd;

    Style();

public:
    struct xml_node_rep {
        struct font_node {
            struct attrs {
                float size;
                std::optional<int> spacing;
            };
            static Result<std::runtime_error, font_node>
            parse_xml(const char*);

            std::string font_name;
            xml::Attribute<attrs> attrs;
        } font;
        struct custom_node {
            custom_parameters_t params { };
            auto unmarshall_self(const pugi::xml_node& n)
                -> Result<std::runtime_error, Unit>;
        } custom;
    };

public:
    Result<std::runtime_error, Style> from_xml(pugi::xml_node&);
    Style(Style&) = delete;
    Style& operator=(Style&) = delete;
    Style(Style&&);
    Style& operator=(Style&&);
    ~Style();

    const ::Font& get_font() const;
    const FontData& get_font_data() const;

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
