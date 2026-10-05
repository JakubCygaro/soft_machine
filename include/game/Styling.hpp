#pragma once

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

class Style {
private:
    std::unordered_map<std::string, StylingParameter> custom_parameters { };

public:
    Style();
    Style(Style&) = delete;
    Style& operator=(Style&) = delete;
    Style(Style&&);
    Style& operator=(Style&&);
    ~Style();

    const ::Font& get_font() const;
    const FontData& get_font_data() const;

    const StylingParameter& get_styling_param(const std::string&) const;
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
        if(std::holds_alternative<T>(param)){
            return std::get_if<T>(&param);
        }

        return std::nullopt;
    }
};
}
