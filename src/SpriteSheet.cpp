#include "SpriteSheet.hpp"

namespace
{
void fillRect(sf::Image& img, int x, int y, int w, int h, sf::Color color)
{
    for (int yy = y; yy < y + h; ++yy)
        for (int xx = x; xx < x + w; ++xx)
            if (xx >= 0 && yy >= 0 && xx < static_cast<int>(img.getSize().x) &&
                yy < static_cast<int>(img.getSize().y))
                img.setPixel({static_cast<unsigned>(xx), static_cast<unsigned>(yy)}, color);
}

// A small hooded traveller, front-facing. Four frames: legs apart
// (stride left), legs together with a 1px bob, legs apart (stride
// right), legs together. At 32x32 the cycle reads through the boots
// and the bob — the oldest trick in sprite animation.
void drawPlayerFrame(sf::Image& img, int originX, int frame)
{
    const sf::Color hood{80, 200, 120};
    const sf::Color hoodDark{52, 156, 88};
    const sf::Color skin{240, 220, 180};
    const sf::Color dark{30, 30, 40};
    const sf::Color trousers{60, 50, 70};
    const sf::Color gold{210, 175, 70};

    const int bob = (frame % 2 == 1) ? -1 : 0;
    const bool apart = (frame % 2 == 0);

    // Hood and face.
    fillRect(img, originX + 14, 1 + bob, 4, 2, hoodDark);
    fillRect(img, originX + 9, 3 + bob, 14, 10, hood);
    fillRect(img, originX + 11, 6 + bob, 10, 7, skin);
    fillRect(img, originX + 13, 9 + bob, 2, 2, dark);
    fillRect(img, originX + 18, 9 + bob, 2, 2, dark);
    // Cloak, belt, arms.
    fillRect(img, originX + 8, 13 + bob, 16, 10, hood);
    fillRect(img, originX + 8, 20 + bob, 16, 3, hoodDark);
    fillRect(img, originX + 8, 18 + bob, 16, 2, dark);
    fillRect(img, originX + 15, 18 + bob, 2, 2, gold);
    fillRect(img, originX + 5, 14 + bob, 3, 8, hoodDark);
    fillRect(img, originX + 24, 14 + bob, 3, 8, hoodDark);
    fillRect(img, originX + 5, 22 + bob, 3, 2, skin);
    fillRect(img, originX + 24, 22 + bob, 3, 2, skin);
    // Legs and boots: the walk itself.
    if (apart)
    {
        fillRect(img, originX + 10, 23, 4, 6, trousers);
        fillRect(img, originX + 18, 23, 4, 6, trousers);
        if (frame == 0)
        {
            fillRect(img, originX + 8, 29, 6, 3, dark);
            fillRect(img, originX + 18, 29, 5, 3, dark);
        }
        else
        {
            fillRect(img, originX + 10, 29, 5, 3, dark);
            fillRect(img, originX + 19, 29, 6, 3, dark);
        }
    }
    else
    {
        fillRect(img, originX + 12, 23, 3, 6, trousers);
        fillRect(img, originX + 17, 23, 3, 6, trousers);
        fillRect(img, originX + 11, 29, 5, 3, dark);
        fillRect(img, originX + 17, 29, 5, 3, dark);
    }
}

void drawCrate(sf::Image& img, int originX)
{
    const sf::Color wood{176, 116, 52};
    const sf::Color woodDark{140, 88, 38};
    const sf::Color edge{110, 68, 30};
    const sf::Color nail{70, 45, 25};

    fillRect(img, originX, 0, 32, 32, wood);
    fillRect(img, originX, 10, 32, 1, woodDark);
    fillRect(img, originX, 21, 32, 1, woodDark);
    // Diagonal brace.
    for (int i = 0; i < 28; ++i)
    {
        fillRect(img, originX + 2 + i, 29 - i, 2, 2, woodDark);
    }
    // Frame and corner nails.
    fillRect(img, originX, 0, 32, 2, edge);
    fillRect(img, originX, 30, 32, 2, edge);
    fillRect(img, originX, 0, 2, 32, edge);
    fillRect(img, originX + 30, 0, 2, 32, edge);
    fillRect(img, originX + 3, 3, 2, 2, nail);
    fillRect(img, originX + 27, 3, 2, 2, nail);
    fillRect(img, originX + 3, 27, 2, 2, nail);
    fillRect(img, originX + 27, 27, 2, 2, nail);
}
} // namespace

sf::Image buildSpriteSheetImage()
{
    sf::Image img({sprites::SHEET_W, sprites::SHEET_H}, sf::Color::Transparent);
    for (int frame = 0; frame < 4; ++frame)
        drawPlayerFrame(img, sprites::PLAYER_X + frame * sprites::FRAME, frame);
    drawCrate(img, sprites::CRATE_X);
    return img;
}
