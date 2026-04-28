
#include "stb_truetype.h"
#include "modules/Graphics/UI/UIBase/Text.h"

void Text::setText(const std::string& text)
{
    _text = text;
    rebuild();
}

void Text::setFont(const std::shared_ptr<UIFont>& font)
{
    _font = font;
    rebuild();
}

void Text::setFontSize(int size)
{
    if (!_font) {
        _font = std::make_shared<UIFont>("res/Roboto-Black.ttf", size);
    }
    else {
        _font->setSize(size);
    }
    rebuild();
}

void Text::rebuild()
{
    auto& tris = _mesh.getTriangles();
    tris.clear();

    if (!_font || _text.empty()) {
        return;
    }

    _mesh.setTexture(_font->getTexture());

    const auto& chars = _font->getChars();
    float cursorX = 0.0f;
    float cursorY = 0.0f;

    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;
    bool hasGlyph = false;

    for (char c : _text) {
        if (c < 32 || c >= 128) continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(
            const_cast<stbtt_bakedchar*>(chars.data()),
            _font->getAtlasWidth(),
            _font->getAtlasHeight(),
            c - 32,
            &cursorX,
            &cursorY,
            &q,
            1
        );

        if (!hasGlyph) {
            minX = q.x0;
            maxX = q.x1;
            minY = q.y0;
            maxY = q.y1;
            hasGlyph = true;
        }
        else {
            minX = std::min(minX, q.x0);
            maxX = std::max(maxX, q.x1);
            minY = std::min(minY, q.y0);
            maxY = std::max(maxY, q.y1);
        }
    }

    if (!hasGlyph) {
        _textWidthPx = 0.0f;
        _textHeightPx = 0.0f;
        return;
    }

    _textWidthPx = maxX - minX;
    _textHeightPx = maxY - minY;

    const float boxWidthPx = static_cast<float>(_transform.getSizePx().x);
    float offsetX = -0.5f * (minX + maxX);
    if (_alignment == Alignment::Center) {
        offsetX = -0.5f * (minX + maxX);
    }
    else if (_alignment == Alignment::Left) {
        offsetX = -0.5f * boxWidthPx + _paddingPx - minX;
    }
    else {
        offsetX = 0.5f * boxWidthPx - _paddingPx - maxX;
    }

    const float offsetY = -0.5f * (minY + maxY);

    cursorX = offsetX;
    cursorY = offsetY;

    const float scaleX = 2.0f / static_cast<float>(CFG_WINDOW_WIDTH);
    const float scaleY = 2.0f / static_cast<float>(CFG_WINDOW_HEIGHT);

    for (char c : _text) {
        if (c < 32 || c >= 128) continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(
            const_cast<stbtt_bakedchar*>(chars.data()),
            _font->getAtlasWidth(),
            _font->getAtlasHeight(),
            c - 32,
            &cursorX,
            &cursorY,
            &q,
            1
        );

        float x0 = q.x0 * scaleX;
        float y0 = -q.y0 * scaleY;
        float x1 = q.x1 * scaleX;
        float y1 = -q.y1 * scaleY;

        Vertex2D v0{ { x0, y0 }, { q.s0, q.t0 }, _color };
        Vertex2D v1{ { x1, y0 }, { q.s1, q.t0 }, _color };
        Vertex2D v2{ { x1, y1 }, { q.s1, q.t1 }, _color };
        Vertex2D v3{ { x0, y1 }, { q.s0, q.t1 }, _color };

        tris.push_back({ v0, v1, v2 });
        tris.push_back({ v0, v2, v3 });
    }
}
