// Text.h
#pragma once

#include "modules/Graphics/UI/UIObject.h"
#include "modules/Graphics/UI/Font.h"
#include "stb_image_write.h"
#include <fstream>
#include <vector>
#include <string>
#include <memory>

class Text : public UIObject {
public:
    Text(std::shared_ptr<Font> font) : _font(font) {}

    using UIObject::UIObject;

    void setFont(std::shared_ptr<Font> font) {
        _font = font;
        rebuildMesh();
    }

    void setText(const std::string& text) {
        _text = text;
        rebuildMesh();
    }

    void setColor(const Color& color) {
        if (_font) {
            _font->setColor(color);
        }
        rebuildMesh();
    }

    // Set position in percentage (0-100% of screen)
    void setPositionPercent(float xPercent, float yPercent) {
        _positionPercent = { xPercent, yPercent };
        _usePercentPositioning = true;
        rebuildMesh();
    }

    // Set position in pixels (for backward compatibility)
    void setPositionPixels(float xPixels, float yPixels) {
        _positionPixels = { xPixels, yPixels };
        _usePercentPositioning = false;
        rebuildMesh();
    }

    // Set anchor point (0-1, default is top-left)
    void setAnchor(float xAnchor, float yAnchor) {
        _anchor = { xAnchor, yAnchor };
        rebuildMesh();
    }

    // Set text alignment
    enum class Alignment { Left, Center, Right };
    void setAlignment(Alignment alignment) {
        _alignment = alignment;
        rebuildMesh();
    }

private:
    void rebuildMesh() {
        if (!_font || !_parentUI || !_parentUI->_window) return;

        _mesh.triangles.clear();

        float screenW = (float)_parentUI->_window->getWidth();
        float screenH = (float)_parentUI->_window->getHeight();

        // Calculate starting position based on positioning mode
        float startX, startY;

        if (_usePercentPositioning) {
            // Convert percentage to pixels
            startX = (_positionPercent.x / 100.0f) * screenW;
            startY = (_positionPercent.y / 100.0f) * screenH;
        }
        else {
            // Use pixel coordinates directly
            startX = _positionPixels.x;
            startY = _positionPixels.y;
        }

        // Apply anchor point
        // First, calculate the total width and height of the text
        float totalWidth = 0;
        float totalHeight = _font->getSize(); // Approximate height

        // Calculate total width by measuring each character
        const auto& chars = _font->getChars();
        int atlasWidth = _font->getAtlasWidth();
        int atlasHeight = _font->getAtlasHeight();

        float x = 0;
        float y = 0;

        // First pass: calculate total width
        for (char c : _text) {
            if (c < 32 || c >= 32 + chars.size()) continue;

            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(
                const_cast<stbtt_bakedchar*>(chars.data()),
                atlasWidth,
                atlasHeight,
                c - 32,
                &x, &y,
                &q,
                1
            );
            totalWidth = x; // x gets updated with the advance
        }

        // Apply alignment offset
        float alignmentOffset = 0;
        switch (_alignment) {
        case Alignment::Center:
            alignmentOffset = -totalWidth / 2;
            break;
        case Alignment::Right:
            alignmentOffset = -totalWidth;
            break;
        case Alignment::Left:
        default:
            alignmentOffset = 0;
            break;
        }

        // Apply anchor offset
        float anchorOffsetX = -totalWidth * _anchor.x;
        float anchorOffsetY = -totalHeight * _anchor.y;

        // Reset position for actual rendering
        x = startX + alignmentOffset + anchorOffsetX;
        y = startY + anchorOffsetY;

        float scaleX = 2.0f / screenW;
        float scaleY = 2.0f / screenH;

        // Second pass: actually render the text
        for (char c : _text) {
            if (c < 32 || c >= 32 + chars.size()) continue;

            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(
                const_cast<stbtt_bakedchar*>(chars.data()),
                atlasWidth,
                atlasHeight,
                c - 32,
                &x, &y,
                &q,
                1
            );

            // Convert from pixel coordinates to NDC
            float x0 = (q.x0) * scaleX - 1.0f;
            float y0 = 1.0f - (q.y0) * scaleY;
            float x1 = (q.x1) * scaleX - 1.0f;
            float y1 = 1.0f - (q.y1) * scaleY;

            // Create vertices
            Vertex v0, v1, v2, v3;

            v0.position = { x0, y0, 0 };
            v1.position = { x1, y0, 0 };
            v2.position = { x1, y1, 0 };
            v3.position = { x0, y1, 0 };

            v0.texCoordinate = { q.s0, q.t0 };
            v1.texCoordinate = { q.s1, q.t0 };
            v2.texCoordinate = { q.s1, q.t1 };
            v3.texCoordinate = { q.s0, q.t1 };

            Color textColor = _font->getColor();
            v0.color = v1.color = v2.color = v3.color = textColor;

            Triangle t1{ v0, v1, v2 };
            Triangle t2{ v0, v2, v3 };

            t1.normal = t2.normal = { 0, 0, -1 };

            _mesh.triangles.push_back(t1);
            _mesh.triangles.push_back(t2);
        }

        _mesh.texture = _font->getTexture();
    }

private:
    std::string _text;
    std::shared_ptr<Font> _font;

    // Positioning
    vec2<float> _positionPercent = { 0, 0 };
    vec2<float> _positionPixels = { 0, 0 };
    vec2<float> _anchor = { 0, 0 }; // 0-1, default top-left
    Alignment _alignment = Alignment::Left;
    bool _usePercentPositioning = true; // Default to percentage
};