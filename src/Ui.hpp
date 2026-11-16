#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Shared screen-space UI widgets (Lesson 80): the pieces every
// menu screen needs — Label (a line of text), Button (a
// clickable, stateful row), Panel (a backing rectangle). They
// replace the runtime's hand-rolled per-screen text blocks, so
// a game's screens are composed from the same objects and a
// style change lands everywhere at once.
//
// The split of labour: widgets own geometry and drawing; hosts
// own input routing. A host feeds each visible Button the
// pointer (setHovered) and asks clickable(point) when a click
// lands; what a click *means* stays the host's business.
// Widgets draw in the target's current view — hosts present
// them in screen space (the default view).
namespace ui
{

class Label
{
public:
    Label(const sf::Font& font, const std::string& text,
          unsigned int size, sf::Color fill);

    void setText(const std::string& text);
    std::string text() const { return m_text.getString(); }
    void setColor(sf::Color fill) { m_text.setFillColor(fill); }
    void setPosition(sf::Vector2f position)
    {
        m_text.setPosition(position);
    }
    sf::Vector2f position() const { return m_text.getPosition(); }
    sf::FloatRect bounds() const { return m_text.getGlobalBounds(); }
    // Places the label so its bounds are centred at (cx, cy) —
    // the runtime's long-standing overlay convention.
    void centreOn(float cx, float cy);
    void draw(sf::RenderTarget& target) { target.draw(m_text); }

private:
    sf::Text m_text;
};

class Button
{
public:
    Button(const sf::Font& font, const std::string& text,
           unsigned int size = 20);

    void setText(const std::string& text);
    std::string text() const { return m_text.getString(); }

    // Geometry and state, set by the host: the rect is the
    // button's whole footprint; `selected` is the keyboard-focus
    // highlight (gold outline); a disabled button draws dimmed
    // and is never clickable.
    sf::FloatRect rect;
    bool enabled = true;
    bool selected = false;

    void setHovered(bool hovered) { m_hovered = hovered; }
    bool hovered() const { return m_hovered; }
    bool contains(sf::Vector2f point) const
    {
        return rect.contains(point);
    }
    bool clickable(sf::Vector2f point) const
    {
        return enabled && contains(point);
    }
    // Centres the rect horizontally on cx, with its top at top.
    void placeCentred(float cx, float top, float width, float height);
    void draw(sf::RenderTarget& target);

private:
    sf::Text m_text;
    bool m_hovered = false;
};

class Panel
{
public:
    sf::FloatRect rect;
    sf::Color fill = sf::Color(8, 8, 18, 170);
    sf::Color outline = sf::Color::Transparent;
    float outlineThickness = 0.f;

    void draw(sf::RenderTarget& target) const;
};

} // namespace ui
