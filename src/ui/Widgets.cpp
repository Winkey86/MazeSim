#include "ui/Widgets.h"
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <cctype>

namespace ml::ui {

static bool TryParseInt(const std::string& s, int& out) {
    if (s.empty()) return false;
    size_t i = 0;
    int sign = 1;
    if (s[0] == '-') {
        sign = -1;
        i = 1;
        if (s.size() == 1) return false;
    }
    long long value = 0;
    for (; i < s.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(s[i]);
        if (!std::isdigit(ch)) return false;
        value = value * 10 + (ch - '0');
        if (value > (long long)std::numeric_limits<int>::max() + 1LL) return false;
    }
    long long signedValue = value * sign;
    if (signedValue < std::numeric_limits<int>::min() || signedValue > std::numeric_limits<int>::max()) {
        return false;
    }
    out = static_cast<int>(signedValue);
    return true;
}

bool FontCache::LoadDefault() {
#ifdef _WIN32
    const char* candidates[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf"
    };
    for (auto* p : candidates) {
        if (std::filesystem::exists(p)) {
            if (m_font.openFromFile(p)) { m_ready = true; return true; }
        }
    }
#endif
    // fallback: try relative
    if (m_font.openFromFile("assets/DejaVuSans.ttf")) { m_ready = true; return true; }
    m_ready = false;
    return false;
}

void Button::Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const {
    sf::RectangleShape r;
    r.setPosition({rect.position.x, rect.position.y});
    r.setSize({rect.size.x, rect.size.y});
    r.setFillColor(enabled ? s.bg : sf::Color(60,60,60,160));
    r.setOutlineColor(s.outline);
    r.setOutlineThickness(1.f);
    rt.draw(r);

    sf::Text t(font, label, 14);
    t.setFillColor(s.text);
    auto b = t.getLocalBounds();
    t.setPosition({rect.position.x + (rect.size.x - b.size.x) * 0.5f, rect.position.y + (rect.size.y - b.size.y) * 0.5f - 2.f});
    rt.draw(t);
}

void Button::Handle(const sf::Event& e, const sf::Vector2f& mouse) {
    if (!enabled) return;
    if (e.is<sf::Event::MouseButtonPressed>()) {
        auto mb = e.getIf<sf::Event::MouseButtonPressed>();
        if (mb && mb->button == sf::Mouse::Button::Left) {
            if (rect.contains(mouse) && onClick) onClick();
        }
    }
}

void Dropdown::Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const {
    sf::RectangleShape r;
    r.setPosition(rect.position);
    r.setSize(rect.size);
    r.setFillColor(s.bg);
    r.setOutlineColor(s.outline);
    r.setOutlineThickness(1.f);
    rt.draw(r);

    std::string shown = (selected >= 0 && selected < (int)items.size()) ? items[(size_t)selected] : "";
    sf::Text t(font, label + ": " + shown, 14);
    t.setFillColor(s.text);
    t.setPosition({rect.position.x + 6.f, rect.position.y + 6.f});
    rt.draw(t);

    if (expanded) {
        sf::RectangleShape box;
        box.setPosition({rect.position.x, rect.position.y + rect.size.y});
        box.setSize({rect.size.x, rect.size.y * (float)items.size()});
        box.setFillColor(s.panel);
        box.setOutlineColor(s.outline);
        box.setOutlineThickness(1.f);
        rt.draw(box);

        for (size_t i = 0; i < items.size(); ++i) {
            sf::Text it(font, items[i], 14);
            it.setFillColor((int)i == selected ? s.accent : s.text);
            it.setPosition({rect.position.x + 8.f, rect.position.y + rect.size.y + rect.size.y * (float)i + 6.f});
            rt.draw(it);
        }
    }
}

void Dropdown::Handle(const sf::Event& e, const sf::Vector2f& mouse) {
    if (e.is<sf::Event::MouseButtonPressed>()) {
        auto mb = e.getIf<sf::Event::MouseButtonPressed>();
        if (mb && mb->button == sf::Mouse::Button::Left) {
            if (rect.contains(mouse)) {
                expanded = !expanded;
                return;
            }
            if (expanded) {
                sf::FloatRect box{ {rect.position.x, rect.position.y + rect.size.y}, {rect.size.x, rect.size.y * (float)items.size()} };
                if (box.contains(mouse)) {
                    int i = (int)((mouse.y - (rect.position.y + rect.size.y)) / rect.size.y);
                    if (i >= 0 && i < (int)items.size()) selected = i;
                }
                expanded = false;
            }
        }
    }
}

void CycleSelector::SetItems(std::vector<std::string> v) {
    items = std::move(v);
    if (items.empty()) {
        selected = 0;
    } else {
        selected = std::clamp(selected, 0, (int)items.size() - 1);
    }
}

void CycleSelector::Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const {
    sf::RectangleShape r;
    r.setPosition(rect.position);
    r.setSize(rect.size);
    r.setFillColor(s.bg);
    r.setOutlineColor(s.outline);
    r.setOutlineThickness(1.f);
    rt.draw(r);

    std::string shown = (selected >= 0 && selected < (int)items.size()) ? items[(size_t)selected] : std::string{};
    sf::Text t(font, label + ": " + shown + "  (click)", 14);
    t.setFillColor(s.text);
    t.setPosition({rect.position.x + 6.f, rect.position.y + 6.f});
    rt.draw(t);
}

void CycleSelector::Handle(const sf::Event& e, const sf::Vector2f& mouse) {
    if (items.empty()) return;
    if (!e.is<sf::Event::MouseButtonPressed>()) return;
    auto mb = e.getIf<sf::Event::MouseButtonPressed>();
    if (!mb) return;
    if (!rect.contains(mouse)) return;

    if (mb->button == sf::Mouse::Button::Left) {
        selected = (selected + 1) % (int)items.size();
    } else if (mb->button == sf::Mouse::Button::Right) {
        selected = (selected - 1);
        if (selected < 0) selected = (int)items.size() - 1;
    }
}

void Toggle::Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const {
    sf::RectangleShape r;
    r.setPosition(rect.position);
    r.setSize(rect.size);
    r.setFillColor(s.bg);
    r.setOutlineColor(s.outline);
    r.setOutlineThickness(1.f);
    rt.draw(r);

    sf::RectangleShape box;
    box.setPosition({rect.position.x + 6.f, rect.position.y + 6.f});
    box.setSize({rect.size.y - 12.f, rect.size.y - 12.f});
    box.setFillColor(value ? s.accent : sf::Color(50,50,50,180));
    box.setOutlineColor(s.outline);
    box.setOutlineThickness(1.f);
    rt.draw(box);

    sf::Text t(font, label, 14);
    t.setFillColor(s.text);
    t.setPosition({rect.position.x + rect.size.y + 6.f, rect.position.y + 6.f});
    rt.draw(t);
}

void Toggle::Handle(const sf::Event& e, const sf::Vector2f& mouse) {
    if (e.is<sf::Event::MouseButtonPressed>()) {
        auto mb = e.getIf<sf::Event::MouseButtonPressed>();
        if (mb && mb->button == sf::Mouse::Button::Left) {
            if (rect.contains(mouse)) value = !value;
        }
    }
}

void SliderInt::Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const {
    sf::RectangleShape r;
    r.setPosition(rect.position);
    r.setSize(rect.size);
    r.setFillColor(s.bg);
    r.setOutlineColor(s.outline);
    r.setOutlineThickness(1.f);
    rt.draw(r);

    sf::Text t(font, label + ": x" + std::to_string(value), 14);
    t.setFillColor(s.text);
    t.setPosition({rect.position.x + 6.f, rect.position.y + 6.f});
    rt.draw(t);

    // bar
    float barY = rect.position.y + rect.size.y - 10.f;
    sf::RectangleShape bar;
    bar.setPosition({rect.position.x + 6.f, barY});
    bar.setSize({rect.size.x - 12.f, 4.f});
    bar.setFillColor(sf::Color(70,70,70,200));
    rt.draw(bar);

    float tNorm = (max == min) ? 0.f : (float)(value - min) / (float)(max - min);
    float knobX = (rect.position.x + 6.f) + (rect.size.x - 12.f) * tNorm;
    sf::CircleShape knob(6.f);
    knob.setPosition({knobX - 6.f, barY - 4.f});
    knob.setFillColor(s.accent);
    rt.draw(knob);
}

void SliderInt::Handle(const sf::Event& e, const sf::Vector2f& mouse) {
    auto barRect = sf::FloatRect{{rect.position.x + 6.f, rect.position.y + rect.size.y - 14.f}, {rect.size.x - 12.f, 14.f}};

    if (e.is<sf::Event::MouseButtonPressed>()) {
        auto mb = e.getIf<sf::Event::MouseButtonPressed>();
        if (mb && mb->button == sf::Mouse::Button::Left) {
            if (barRect.contains(mouse)) dragging = true;
        }
    }
    if (e.is<sf::Event::MouseButtonReleased>()) {
        auto mb = e.getIf<sf::Event::MouseButtonReleased>();
        if (mb && mb->button == sf::Mouse::Button::Left) dragging = false;
    }
    if (e.is<sf::Event::MouseMoved>() && dragging) {
        float x0 = barRect.position.x;
        float x1 = barRect.position.x + barRect.size.x;
        float t = (mouse.x - x0) / (x1 - x0);
        t = std::min(1.f, std::max(0.f, t));
        value = min + (int)std::round(t * (float)(max - min));
    }
}

void StepperInt::Draw(sf::RenderTarget& rt, const sf::Font& font, const Style& s) const {
    sf::RectangleShape r;
    r.setPosition(rect.position);
    r.setSize(rect.size);
    r.setFillColor(s.bg);
    r.setOutlineColor(s.outline);
    r.setOutlineThickness(1.f);
    rt.draw(r);

    std::string shown = editing ? editBuffer : std::to_string(value);
    if (editing) shown += "_";
    sf::Text t(font, label + ": " + shown, 14);
    t.setFillColor(editing ? s.accent : s.text);
    t.setPosition({rect.position.x + 6.f, rect.position.y + 6.f});
    rt.draw(t);

    // + and - buttons
    sf::RectangleShape minus;
    minus.setPosition({rect.position.x + rect.size.x - 56.f, rect.position.y + 6.f});
    minus.setSize({22.f, rect.size.y - 12.f});
    minus.setFillColor(sf::Color(60,60,60,180));
    minus.setOutlineColor(s.outline);
    minus.setOutlineThickness(1.f);
    rt.draw(minus);

    sf::RectangleShape plus;
    plus.setPosition({rect.position.x + rect.size.x - 28.f, rect.position.y + 6.f});
    plus.setSize({22.f, rect.size.y - 12.f});
    plus.setFillColor(sf::Color(60,60,60,180));
    plus.setOutlineColor(s.outline);
    plus.setOutlineThickness(1.f);
    rt.draw(plus);

    sf::Text tminus(font, "-", 16);
    tminus.setFillColor(s.text);
    tminus.setPosition({minus.getPosition().x + 7.f, minus.getPosition().y + 4.f});
    rt.draw(tminus);

    sf::Text tplus(font, "+", 16);
    tplus.setFillColor(s.text);
    tplus.setPosition({plus.getPosition().x + 6.f, plus.getPosition().y + 2.f});
    rt.draw(tplus);
}

void StepperInt::Handle(const sf::Event& e, const sf::Vector2f& mouse) {
    sf::FloatRect minus{{rect.position.x + rect.size.x - 56.f, rect.position.y + 6.f}, {22.f, rect.size.y - 12.f}};
    sf::FloatRect plus{{rect.position.x + rect.size.x - 28.f, rect.position.y + 6.f}, {22.f, rect.size.y - 12.f}};

    auto commitEdit = [&] {
        if (!editing) return;
        int parsed = 0;
        if (TryParseInt(editBuffer, parsed)) {
            value = std::clamp(parsed, min, max);
        }
        editing = false;
        editFresh = false;
        editBuffer.clear();
    };
    auto cancelEdit = [&] {
        if (!editing) return;
        editing = false;
        editFresh = false;
        editBuffer.clear();
    };

    if (e.is<sf::Event::MouseButtonPressed>()) {
        auto mb = e.getIf<sf::Event::MouseButtonPressed>();
        if (mb && mb->button == sf::Mouse::Button::Left) {
            if (minus.contains(mouse)) {
                commitEdit();
                value = std::max(min, value - step);
                return;
            }
            if (plus.contains(mouse)) {
                commitEdit();
                value = std::min(max, value + step);
                return;
            }
            if (rect.contains(mouse)) {
                if (!editing) {
                    editing = true;
                    editFresh = true;
                    editBuffer = std::to_string(value);
                }
                return;
            }
            if (editing) commitEdit();
        }
    }

    if (!editing) return;

    if (e.is<sf::Event::KeyPressed>()) {
        auto kp = e.getIf<sf::Event::KeyPressed>();
        if (kp && kp->code == sf::Keyboard::Key::Enter) {
            commitEdit();
            return;
        }
        if (kp && kp->code == sf::Keyboard::Key::Escape) {
            cancelEdit();
            return;
        }
    }

    if (e.is<sf::Event::TextEntered>()) {
        auto te = e.getIf<sf::Event::TextEntered>();
        if (!te) return;
        char32_t c = te->unicode;
        if (c == 8) { // backspace
            if (editFresh) {
                editBuffer.clear();
                editFresh = false;
            } else if (!editBuffer.empty()) {
                editBuffer.pop_back();
            }
            return;
        }
        if (c < 32 || c > 126) return;
        if (editFresh) {
            editBuffer.clear();
            editFresh = false;
        }
        if (c == '-' && min < 0) {
            if (editBuffer.empty()) editBuffer.push_back('-');
            return;
        }
        if (c >= '0' && c <= '9') {
            editBuffer.push_back(static_cast<char>(c));
        }
    }
}

} // namespace ml::ui
