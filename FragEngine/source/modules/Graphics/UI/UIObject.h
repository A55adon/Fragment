#pragma once

#include "core/DataTypes.h"
#include "core/Mesh.h"
#include "modules/Graphics/Window.h"
#include "stb_truetype.h"
#include <glad/glad.h>
#include <vector>
#include <functional>

struct UITransform {
    vec2<float> position = vec2<float>(50.f, 50.f);
    vec2<float> size = vec2<float>(10.f, 5.f);
    float rotation = 0.f;
};

struct UIStyle {
    float cornerRoundness = 2.f;
    int cornerSegments = 5;
    Color baseColor = Color::fromHex("#262624");
    Color textColor = Color::fromHex("#CBCAC7");
    Color secondaryColor = Color::fromHex("#2B2B29");

    float borderWidth = .5f;
    Color borderColor = Color::fromHex("#454542");
};

enum ResizeEdge : int {
    EDGE_NONE = 0,
    EDGE_LEFT = 1 << 0,
    EDGE_RIGHT = 1 << 1,
    EDGE_TOP = 1 << 2,
    EDGE_BOTTOM = 1 << 3,
};

class UI;

class UIObject {
public:
    UIObject() = default;
    UIObject(UI* parentUI) : _parentUI(parentUI), _ID(nextID++) {}

    bool operator==(const UIObject& other) const {
        return this->_ID == other._ID;
    }

    // Setters & Getters
    int getID() { return _ID; }
    void setName(std::string name) { _name = name; }
    std::string getName() { return _name; }

    void setParentUI(UI* parentUI) { _parentUI = parentUI; }

    void setMesh(Mesh mesh) { _mesh = mesh; }
    Mesh& getMesh() { return _mesh; }

    void setStyle(UIStyle style) { _style = style; }
    UIStyle& getStyle() { return _style; }

    void setTransform(UITransform tr) { _transform = tr; }
    UITransform& getTransform() { return _transform; }

    void setOnClick(std::function<void()> onClick) { _onClick = onClick; }

    // OnClick handeling
    bool handleClickRecursive(float mouseX, float mouseY, vec2<float> parentOffset = { 0.0f, 0.0f });

    // Returns (xMin, xMax, yMin, yMax) in screen pixels.
    vec4<float> getBoundsPixels(vec2<float> worldPos = { 0.0f, 0.0f });

    std::vector<std::unique_ptr<UIObject>>& getChildren() { return _children; }


    bool isDraggable = false;
    bool isResizable = false;


    virtual void onDrag(float dx, float dy);

    virtual void onResize(float dx, float dy, int edgeMask);

    int hitTest(float mouseX, float mouseY, vec2<float> worldPos = { 0.f, 0.f });

    UIObject* findInteractionTarget(float mouseX, float mouseY, vec2<float> parentOffset, int& outEdgeMask);

    void setOnDrag(std::function<void(float, float)> fn) { _onDrag = fn; }

private:
    inline static int nextID = 0;
    int _ID = -1;
protected:
    std::string _name;

    std::function<void(float, float)> _onDrag;

    Mesh _mesh;
    UITransform _transform;
    UIStyle _style;

    std::vector<std::unique_ptr<UIObject>> _children;
    UI* _parentUI;

    std::function<void()> _onClick;

    static constexpr float RESIZE_HANDLE_PX = 8.f;
    static constexpr float MIN_SIZE = 0.5f;

};