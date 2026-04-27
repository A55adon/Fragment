#include "UIObject.h"
#include "modules/Graphics/UI/UI.h"
#include "modules/IO/UserInput/Input.h"
#include "modules/Graphics/Window.h"
#include "modules/Graphics/UI/UITypes.h"

#include <map>
#include <array>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "earcut.hpp"

vec4<float> UIObject::getBoundsPixels(vec2<float> worldPos)
{
    int width, height;
    glfwGetWindowSize(_parentUI->_window->getRawWindow(), &width, &height);

    // worldPos is in 0-100 space — convert to NDC offset
    float ndcOffsetX = (worldPos.x / 100.0f) * 2.0f - 1.0f;
    float ndcOffsetY = 1.0f - (worldPos.y / 100.0f) * 2.0f;

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto& tri : _mesh.triangles)
    {
        for (const auto& v : tri.vertices)
        {
            // Mesh verts are origin-centered NDC, offset by converted world pos
            float ndcX = v.position.x + ndcOffsetX;
            float ndcY = v.position.y + ndcOffsetY;

            float px = (ndcX + 1.0f) * 0.5f * width;
            float py = (1.0f - (ndcY + 1.0f) * 0.5f) * height;

            if (px < minX) minX = px;
            if (px > maxX) maxX = px;
            if (py < minY) minY = py;
            if (py > maxY) maxY = py;
        }
    }

    return vec4<float>(minX, maxX, minY, maxY);
}

bool UIObject::handleClickRecursive(float mouseX, float mouseY, vec2<float> parentOffset)
{
    vec2<float> worldPos = parentOffset + _transform.position;

    // Check children first (reverse = topmost drawn on top)
    for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
        if ((*it)->handleClickRecursive(mouseX, mouseY, worldPos))
            return true;
    }

    // Check self
    auto b = getBoundsPixels(worldPos);
    if (mouseX >= b.x && mouseX <= b.y &&
        mouseY >= b.z && mouseY <= b.w)
    {
        if (_onClick) {
            _onClick();
            return true;  // only consume if we actually handled it
        }
        // Hit the bounds but no handler — let it bubble up to parent
        return false;
    }

    return false;
}

void UIObject::onDrag(float dx, float dy) {
    if (_onDrag) _onDrag(dx, dy);
}

void UIObject::onResize(float dx, float dy, int edgeMask)
{
    float aspectFix = 1.f;
    if (_parentUI && _parentUI->_window) {
        float w = (float)_parentUI->_window->getWidth();
        float h = (float)_parentUI->_window->getHeight();
        if (w > 0.f) aspectFix = h / w;
    }

    if (edgeMask & EDGE_LEFT) {
        _transform.position.x += dx / 2.f;                                         
        _transform.size.x = std::max(MIN_SIZE, _transform.size.x - dx / aspectFix);
    }
    if (edgeMask & EDGE_RIGHT) {
        _transform.position.x += dx / 2.f;                                         
        _transform.size.x = std::max(MIN_SIZE, _transform.size.x + dx / aspectFix);
    }
    if (edgeMask & EDGE_TOP) {
        _transform.position.y += dy / 2.f;
        _transform.size.y = std::max(MIN_SIZE, _transform.size.y - dy);
    }
    if (edgeMask & EDGE_BOTTOM) {
        _transform.position.y += dy / 2.f;
        _transform.size.y = std::max(MIN_SIZE, _transform.size.y + dy);
    }
}

int UIObject::hitTest(float mouseX, float mouseY, vec2<float> worldPos)
{
    if (_mesh.triangles.empty()) return 0;

    auto b = getBoundsPixels(worldPos); // (xMin, xMax, yMin, yMax)

    // Complete miss
    if (mouseX < b.x || mouseX > b.y || mouseY < b.z || mouseY > b.w)
        return 0;

    // Interior hit when not resizable — caller decides whether to drag
    if (!isResizable)
        return -1;

    // Classify edge zones
    int mask = EDGE_NONE;
    if (mouseX <= b.x + RESIZE_HANDLE_PX) mask |= EDGE_LEFT;
    if (mouseX >= b.y - RESIZE_HANDLE_PX) mask |= EDGE_RIGHT;
    if (mouseY <= b.z + RESIZE_HANDLE_PX) mask |= EDGE_TOP;
    if (mouseY >= b.w - RESIZE_HANDLE_PX) mask |= EDGE_BOTTOM;

    return (mask != EDGE_NONE) ? mask : -1; // -1 = interior
}

UIObject* UIObject::findInteractionTarget(float mouseX, float mouseY,
    vec2<float> parentOffset, int& outEdgeMask)
{
    vec2<float> worldPos = parentOffset + _transform.position;

    // Children first (reverse = topmost rendered last → checked first)
    for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
        UIObject* found = (*it)->findInteractionTarget(mouseX, mouseY, worldPos, outEdgeMask);
        if (found) return found;
    }

    int hit = hitTest(mouseX, mouseY, worldPos);
    if (hit == 0) return nullptr; // complete miss

    if (hit != -1 && isResizable) {
        // Edge zone and resizing is enabled → resize target
        outEdgeMask = hit;
        return this;
    }

    if (isDraggable) {
        // Interior zone (or edge zone on a non-resizable object) → drag target
        outEdgeMask = EDGE_NONE;
        return this;
    }

    return nullptr; // hit the object but it has no interaction enabled
}