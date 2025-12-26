#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>

namespace ml::ui {

struct Style {
    sf::Color bg{sf::Color(30,30,30,220)};
    sf::Color panel{sf::Color(20,20,20,230)};
    sf::Color outline{sf::Color(80,80,80,200)};
    sf::Color text{sf::Color::White};
    sf::Color accent{sf::Color(120,120,255,220)};
};

class FontCache {
public:
    bool LoadDefault();
    const sf::Font& Get() const { return m_font; }
    bool IsReady() const { return m_ready; }

private:
    sf::Font m_font;
    bool m_ready{false};
};

class Button {
public:
    sf::FloatRect rect{};
    std::string label;
    bool enabled{true};
    std::function<void()> onClick;

    void Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const;
    void Handle(const sf::Event& e, const sf::Vector2f& mouse);
};

class Dropdown {
public:
    sf::FloatRect rect{};
    std::vector<std::string> items;
    int selected{0};
    bool expanded{false};
    std::string label;

    void Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const;
    void Handle(const sf::Event& e, const sf::Vector2f& mouse);
};

// Simple click-to-cycle selector (no dropdown list).
// Left click: next item, Right click: previous item.
class CycleSelector {
public:
    sf::FloatRect rect{};
    std::vector<std::string> items;
    int selected{0};
    std::string label;

    void SetItems(std::vector<std::string> v);

    void Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const;
    void Handle(const sf::Event& e, const sf::Vector2f& mouse);
};

class Toggle {
public:
    sf::FloatRect rect{};
    std::string label;
    bool value{false};

    void Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const;
    void Handle(const sf::Event& e, const sf::Vector2f& mouse);
};

class SliderInt {
public:
    sf::FloatRect rect{};
    std::string label;
    int min{1}, max{50};
    int value{1};
    bool dragging{false};

    void Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const;
    void Handle(const sf::Event& e, const sf::Vector2f& mouse);
};

class StepperInt {
public:
    sf::FloatRect rect{};
    std::string label;
    int min{10}, max{200};
    int value{51};
    int step{1};

    void Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const;
    void Handle(const sf::Event& e, const sf::Vector2f& mouse);
    bool IsEditing() const { return editing; }

private:
    bool editing{false};
    bool editFresh{false};
    std::string editBuffer;
};

} // namespace ml::ui
