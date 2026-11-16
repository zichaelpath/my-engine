#include "Ui.hpp"

namespace ui
{

namespace
{
// The engine's house text style (from the runtime's original
// makeText): a black outline, heavier on display sizes.
void styleText(sf::Text& text, unsigned int size, sf::Color fill)
{
    text.setFillColor(fill);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(size > 30 ? 3.f : 2.f);
}
} // namespace

Label::Label(const sf::Font& font, const std::string& text,
             unsigned int size, sf::Color fill)
    : m_text(font, text, size)
{
    styleText(m_text, size, fill);
}

void Label::setText(const std::string& text)
{
    m_text.setString(text);
}

void Label::centreOn(float cx, float cy)
{
    const sf::FloatRect b = m_text.getLocalBounds();
    m_text.setOrigin(
        {b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    m_text.setPosition({cx, cy});
}

Button::Button(const sf::Font& font, const std::string& text,
               unsigned int size)
    : m_text(font, text, size)
{
    styleText(m_text, size, sf::Color::White);
}

void Button::setText(const std::string& text)
{
    m_text.setString(text);
}

void Button::placeCentred(float cx, float top, float width, float height)
{
    rect = {{cx - width / 2.f, top}, {width, height}};
}

void Button::draw(sf::RenderTarget& target)
{
    // State decides the colours: disabled is dimmed out, hover
    // brightens, selected trades brightness for the gold ring.
    sf::Color fill(38, 42, 58, 230);
    sf::Color textFill = sf::Color::White;
    sf::Color outline = sf::Color(90, 98, 120, 255);
    float outlineWidth = 1.f;
    if (!enabled)
    {
        fill = sf::Color(28, 30, 38, 200);
        textFill = sf::Color(120, 120, 120);
        outline = sf::Color(60, 62, 70, 255);
    }
    else if (selected)
    {
        fill = sf::Color(52, 58, 80, 240);
        outline = sf::Color(255, 215, 80);
        outlineWidth = 2.f;
    }
    else if (m_hovered)
    {
        fill = sf::Color(74, 82, 110, 240);
    }

    sf::RectangleShape box(rect.size);
    box.setPosition(rect.position);
    box.setFillColor(fill);
    box.setOutlineColor(outline);
    box.setOutlineThickness(outlineWidth);
    target.draw(box);

    m_text.setFillColor(textFill);
    const sf::FloatRect b = m_text.getLocalBounds();
    m_text.setOrigin(
        {b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    m_text.setPosition(rect.getCenter());
    target.draw(m_text);
}

void Panel::draw(sf::RenderTarget& target) const
{
    sf::RectangleShape box(rect.size);
    box.setPosition(rect.position);
    box.setFillColor(fill);
    box.setOutlineColor(outline);
    box.setOutlineThickness(outlineThickness);
    target.draw(box);
}

} // namespace ui
