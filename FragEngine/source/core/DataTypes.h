#pragma once

#include <string>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>
#include <optional>
#include <format>
#include <concepts>
#include <iostream>

#include "core/config.h"

enum class EGameState {
    UI,
    GAME
};

// Forward-declare mat3 so vec3 methods can reference it if needed
struct mat3;


// =============================================================================
//  vec3
// =============================================================================
template<typename T>
struct vec3
{
    vec3() : x(0), y(0), z(0) {}
    vec3(T nx, T ny, T nz) : x(nx), y(ny), z(nz) {}
    T x, y, z;

    vec3 operator*(T scalar) const { return { x * scalar, y * scalar, z * scalar }; }
    vec3 operator/(T scalar) const { return { x / scalar, y / scalar, z / scalar }; }
    vec3 operator+(const vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    vec3 operator-(const vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    vec3 operator-() const { return { -x, -y, -z }; }

    vec3& operator+=(const vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    vec3& operator-=(const vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    vec3& operator*=(const vec3& o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
    vec3& operator*=(float s) { x *= s;   y *= s;   z *= s;   return *this; }

    bool operator==(const vec3& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const vec3& o) const { return !(*this == o); }

    friend std::ostream& operator<<(std::ostream& os, const vec3<T>& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    T dot(const vec3& o) const { return x * o.x + y * o.y + z * o.z; }

    vec3 cross(const vec3& o) const {
        return { y * o.z - z * o.y,
                 z * o.x - x * o.z,
                 x * o.y - y * o.x };
    }

    T length() const {
        return static_cast<T>(std::sqrt(static_cast<double>(x * x + y * y + z * z)));
    }

    T lengthSquared() const { return x * x + y * y + z * z; }

    vec3 normalized() const {
        T len = length();
        if (len < static_cast<T>(1e-8)) return {};
        return { x / len, y / len, z / len };
    }

    std::string to_string() const {
        return "(" + std::to_string(x) + ", "
            + std::to_string(y) + ", "
            + std::to_string(z) + ")";
    }
};


// =============================================================================
//  vec2
// =============================================================================
template<typename T>
struct vec2
{
    vec2() : x(0), y(0) {}
    vec2(T nx, T ny) : x(nx), y(ny) {}
    T x, y;

    friend std::ostream& operator<<(std::ostream& os, const vec2<T>& v) {
        os << "(" << v.x << ", " << v.y << ", " << ")";
        return os;
    }

    vec2<T> operator+(const vec2<T>& other) const {
        return vec2<T>{x + other.x, y + other.y};
    }

    bool operator==(const vec2<T>& other) const {
        return x == other.x && y == other.y;
    }
};


// =============================================================================
//  vec4
// =============================================================================
template<typename T>
struct vec4
{
    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(T nx, T ny, T nz, T nw) : x(nx), y(ny), z(nz), w(nw) {}
    T x, y, z, w;

    friend std::ostream& operator<<(std::ostream& os, const vec4<T>& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }


    vec4 operator*(T scalar) const { return { x * scalar, y * scalar, z * scalar, w * scalar }; }
    vec4 operator+(const vec4& o) const { return { x + o.x, y + o.y, z + o.z, w + o.w }; }
    vec4 operator-(const vec4& o) const { return { x - o.x, y - o.y, z - o.z, w - o.w }; }
};


// =============================================================================
//  Color
// =============================================================================
struct Color
{
    constexpr Color() : r(0), g(0), b(0), a(1.f) {}
    constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

    float r, g, b, a;

    inline static Color fromHex(const std::string& hex) {
        std::string h = hex[0] == '#' ? hex.substr(1) : hex;
        unsigned int r = std::stoul(h.substr(0, 2), nullptr, 16);
        unsigned int g = std::stoul(h.substr(2, 2), nullptr, 16);
        unsigned int b = std::stoul(h.substr(4, 2), nullptr, 16);
        return Color(r / 255.f, g / 255.f, b / 255.f, 1.f);
    }

    inline std::string toHex() const {
        auto toHexChannel = [](float v) -> std::string {
            int i = static_cast<int>(std::clamp(v, 0.f, 1.f) * 255.f);
            std::stringstream ss;
            ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << i;
            return ss.str();
            };
        return toHexChannel(r) + toHexChannel(g) + toHexChannel(b);
    }

    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color White;
    static const Color Black;
};

inline const Color Color::Red{ 1.f, 0.f, 0.f, 1.f };
inline const Color Color::Green{ 0.f, 1.f, 0.f, 1.f };
inline const Color Color::Blue{ 0.f, 0.f, 1.f, 1.f };
inline const Color Color::White{ 1.f, 1.f, 1.f, 1.f };
inline const Color Color::Black{ 0.f, 0.f, 0.f, 1.f };


struct Vertex
{
    vec3<float> position;
    vec2<float> texCoordinate;
    Color       color;
};

struct Triangle
{
    Vertex      vertices[3];
    vec3<float> normal;
};


struct quat
{
    float x, y, z, w;

    quat() : x(0), y(0), z(0), w(1) {}
    quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    static quat identity() { return { 0, 0, 0, 1 }; }

    static quat axisAngle(const vec3<float>& axis, float angle)
    {
        float half = angle * 0.5f;
        float s = std::sinf(half);
        return { axis.x * s, axis.y * s, axis.z * s, std::cosf(half) };
    }

    quat operator*(const quat& q) const {
        return {
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        };
    }

    quat operator*(float s) const { return { x * s, y * s, z * s, w * s }; }
    quat operator+(const quat& q) const { return { x + q.x, y + q.y, z + q.z, w + q.w }; }
    quat& operator+=(const quat& q) { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }

    void normalize()
    {
        float len = std::sqrtf(x * x + y * y + z * z + w * w);
        if (len > 1e-8f) { x /= len; y /= len; z /= len; w /= len; }
    }
};


// =============================================================================
//  mat4
// =============================================================================
struct mat4
{
    float v[4][4];

    mat4() { for (auto& row : v) for (auto& e : row) e = 0.0f; }

    static mat4 identity() {
        mat4 m;
        m.v[0][0] = m.v[1][1] = m.v[2][2] = m.v[3][3] = 1.0f;
        return m;
    }

    static mat4 translate(vec3<float> t) {
        mat4 m = identity();
        m.v[0][3] = t.x; m.v[1][3] = t.y; m.v[2][3] = t.z;
        return m;
    }

    static mat4 scale(vec3<float> s) {
        mat4 m = identity();
        m.v[0][0] = s.x; m.v[1][1] = s.y; m.v[2][2] = s.z;
        return m;
    }

    static mat4 rotateX(float rad) {
        mat4 m = identity();
        float c = std::cosf(rad), s = std::sinf(rad);
        m.v[1][1] = c; m.v[1][2] = s; m.v[2][1] = -s; m.v[2][2] = c;
        return m;
    }

    static mat4 rotateY(float rad) {
        mat4 m = identity();
        float c = std::cosf(rad), s = std::sinf(rad);
        m.v[0][0] = c; m.v[0][2] = -s; m.v[2][0] = s; m.v[2][2] = c;
        return m;
    }

    static mat4 rotateZ(float rad) {
        mat4 m = identity();
        float c = std::cosf(rad), s = std::sinf(rad);
        m.v[0][0] = c; m.v[0][1] = s; m.v[1][0] = -s; m.v[1][1] = c;
        return m;
    }

    static mat4 perspective(float fovRad, float aspect, float near, float far) {
        mat4 m;
        float t = std::tanf(fovRad * 0.5f);
        m.v[0][0] = 1.0f / (aspect * t);
        m.v[1][1] = 1.0f / t;
        m.v[2][2] = -(far + near) / (far - near);
        m.v[2][3] = -(2.0f * far * near) / (far - near);
        m.v[3][2] = -1.0f;
        return m;
    }

    mat4 operator*(const mat4& o) const {
        mat4 r;
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                for (int k = 0; k < 4; ++k)
                    r.v[row][col] += v[row][k] * o.v[k][col];
        return r;
    }

    static mat4 fromQuaternion(const quat& q) {
        mat4 m = identity();
        float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
        float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
        m.v[0][0] = 1 - 2 * (yy + zz); m.v[0][1] = 2 * (xy - wz);  m.v[0][2] = 2 * (xz + wy);
        m.v[1][0] = 2 * (xy + wz);  m.v[1][1] = 1 - 2 * (xx + zz); m.v[1][2] = 2 * (yz - wx);
        m.v[2][0] = 2 * (xz - wy);  m.v[2][1] = 2 * (yz + wx);   m.v[2][2] = 1 - 2 * (xx + yy);
        return m;
    }

    static mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
        mat4 m = identity();
        m.v[0][0] = 2.0f / (right - left);
        m.v[1][1] = 2.0f / (top - bottom);
        m.v[2][2] = -2.0f / (far - near);
        m.v[0][3] = -(right + left) / (right - left);
        m.v[1][3] = -(top + bottom) / (top - bottom);
        m.v[2][3] = -(far + near) / (far - near);
        return m;
    }

    static mat4 lookAt(vec3<float> eye, vec3<float> target, vec3<float> up) {
        vec3<float> f = (target - eye).normalized();
        vec3<float> r = f.cross(up).normalized();
        vec3<float> u = r.cross(f);

        mat4 m = identity();
        m.v[0][0] = r.x; m.v[0][1] = r.y; m.v[0][2] = r.z; m.v[0][3] = -r.dot(eye);
        m.v[1][0] = u.x; m.v[1][1] = u.y; m.v[1][2] = u.z; m.v[1][3] = -u.dot(eye);
        m.v[2][0] = -f.x; m.v[2][1] = -f.y; m.v[2][2] = -f.z; m.v[2][3] = f.dot(eye);
        m.v[3][3] = 1.0f;
        return m;
    }
};


// =============================================================================
//  mat3  —  column-major 3×3 matrix
//
//  Columns are the body-frame basis vectors expressed in world space.
//  M * v  transforms a vector from body space to world space.
// =============================================================================
struct mat3
{
    float m[3][3];

    // Default-constructs to identity
    mat3() {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    // Strip the rotation from a mat4 into a column-major mat3.
    //
    // mat4::fromQuaternion stores the matrix in ROW-major layout:
    //   src.v[row][col]
    //
    // This mat3 is COLUMN-major: m[row][col] where COLUMNS are the
    // body-frame basis vectors in world space (standard physics convention).
    // For a pure rotation matrix the column-major form equals the
    // row-major TRANSPOSE, so we store src.v[col][row] (swap indices).
    //
    // Without this swap, getWorldInertiaTensorInverse() computes
    //   R^T * I_body^-1 * R   (wrong)
    // instead of
    //   R   * I_body^-1 * R^T (correct)
    // causing the inertia tensor to be wrong and angular impulses to
    // accelerate rotation instead of damping it — objects visually grow.
    explicit mat3(const mat4& src) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = src.v[j][i];   // transposed: col-major from row-major source
    }

    // ── Factories ─────────────────────────────────────────────────────────────
    static mat3 identity() { return {}; }   // default ctor already is identity

    static mat3 zero() {
        mat3 r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = 0.0f;
        return r;
    }

    // Skew-symmetric [v]× such that  [v]× u == v.cross(u)
    static mat3 skewSymmetric(const vec3<float>& v) {
        mat3 r = zero();
        r.m[0][1] = -v.z;  r.m[0][2] = v.y;
        r.m[1][0] = v.z;  r.m[1][2] = -v.x;
        r.m[2][0] = -v.y;  r.m[2][1] = v.x;
        return r;
    }

    // ── Arithmetic ─────────────────────────────────────────────────────────────
    vec3<float> operator*(const vec3<float>& v) const {
        return {
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        };
    }

    mat3 operator*(const mat3& o) const {
        mat3 r = zero();
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                for (int k = 0; k < 3; ++k)
                    r.m[i][j] += m[i][k] * o.m[k][j];
        return r;
    }

    mat3 operator*(float s) const {
        mat3 r = zero();
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = m[i][j] * s;
        return r;
    }

    mat3 operator+(const mat3& o) const {
        mat3 r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = m[i][j] + o.m[i][j];
        return r;
    }

    mat3& operator+=(const mat3& o) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] += o.m[i][j];
        return *this;
    }

    mat3 operator-(const mat3& o) const {
        mat3 r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = m[i][j] - o.m[i][j];
        return r;
    }

    // ── Linear algebra ──────────────────────────────────────────────────────────
    mat3 transpose() const {
        mat3 r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = m[j][i];
        return r;
    }

    float determinant() const {
        return  m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
            - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
            + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }

    mat3 inverse() const {
        float det = determinant();
        if (std::fabs(det) < 1e-8f) return zero();
        float inv = 1.0f / det;
        mat3 r = zero();
        r.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * inv;
        r.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * inv;
        r.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * inv;
        r.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * inv;
        r.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * inv;
        r.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * inv;
        r.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * inv;
        r.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * inv;
        r.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * inv;
        return r;
    }

    // Gram-Schmidt orthonormalization — prevents float drift from accumulating
    // in the orientation matrix. Call once per physics frame.
    mat3 orthonormalize() const {
        vec3<float> xAxis = { m[0][0], m[1][0], m[2][0] };
        vec3<float> yAxis = { m[0][1], m[1][1], m[2][1] };

        xAxis = xAxis.normalized();
        // Remove the component of y that points along x, then normalize
        float yDotX = yAxis.dot(xAxis);
        yAxis = vec3<float>{
            yAxis.x - xAxis.x * yDotX,
            yAxis.y - xAxis.y * yDotX,
            yAxis.z - xAxis.z * yDotX
        }.normalized();
        vec3<float> zAxis = xAxis.cross(yAxis);

        mat3 r;
        r.m[0][0] = xAxis.x; r.m[1][0] = xAxis.y; r.m[2][0] = xAxis.z;
        r.m[0][1] = yAxis.x; r.m[1][1] = yAxis.y; r.m[2][1] = yAxis.z;
        r.m[0][2] = zAxis.x; r.m[1][2] = zAxis.y; r.m[2][2] = zAxis.z;
        return r;
    }
};

template<typename T>
concept HasIDAndName =
std::default_initializable<T> &&
    requires(T a, const std::string & name) {
        { a.getID() } -> std::convertible_to<int>;
        { a.getName() } -> std::convertible_to<std::string>;
        a.setName(name);
};
template<HasIDAndName T>
class Register {
public:
    Register() = default;

    // Unique ownershipno copying
    Register(const Register&) = delete;
    Register& operator=(const Register&) = delete;

    Register(Register&&) = default;
    Register& operator=(Register&&) = default;

    template<typename... Args>
    T* createNew(const std::string& name, Args&&... args) {
        auto uptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = uptr.get();

        ptr->setName(name);

        size_t index = objects.size();
        idMap[ptr->getID()] = index;
        nameMap[ptr->getName()] = index;

        objects.push_back(std::move(uptr));
        return ptr;
    }

    T* addNew(std::unique_ptr<T> obj) {
        T* ptr = obj.get();

        size_t index = objects.size();
        idMap[ptr->getID()] = index;
        nameMap[ptr->getName()] = index;

        objects.push_back(std::move(obj));
        return ptr;
    }

    T* getByID(int id) {
        auto it = idMap.find(id);
        if (it == idMap.end()) {
            std::cout << "No entry found for ID: " << id << std::endl;
            return nullptr;
        }
        return objects[it->second].get();
    }

    T* getByName(const std::string& name) {
        auto it = nameMap.find(name);
        if (it == nameMap.end()) { 
            std::cout << "No entry found for Name: " << name << std::endl;
            return nullptr; 
        }

        return objects[it->second].get();
    }

    // Returns ownership to the caller
    std::unique_ptr<T> removeByID(int id) {
        size_t index = idMap.at(id);
        std::unique_ptr<T> removed = std::move(objects[index]);

        size_t lastIndex = objects.size() - 1;
        if (index != lastIndex) {
            objects[index] = std::move(objects[lastIndex]);
            idMap[objects[index]->getID()] = index;
            nameMap[objects[index]->getName()] = index;
        }

        objects.pop_back();
        idMap.erase(id);
        nameMap.erase(removed->getName());

        return removed;
    }

    void reset() {
        objects.clear();
        idMap.clear();
        nameMap.clear();
    }

    std::vector<std::unique_ptr<T>>& getAll() { return objects; }

private:
    std::vector<std::unique_ptr<T>> objects;
    std::unordered_map<int, size_t>         idMap;
    std::unordered_map<std::string, size_t> nameMap;
};



struct Transform {
private:
    vec2<float> _position{ 0.0f, 0.0f };
    vec2<float> _size{ 0.0f, 0.0f };

public:
    void setPositionPx(vec2<int> pos) {
        _position.x = (static_cast<float>(pos.x) / CFG_WINDOW_WIDTH) * 2.0f - 1.0f;
        _position.y = (static_cast<float>(pos.y) / CFG_WINDOW_HEIGHT) * 2.0f - 1.0f;
    }

    vec2<int> getPositionPx() const {
        return {
            static_cast<int>((_position.x + 1.0f) * 0.5f * CFG_WINDOW_WIDTH),
            static_cast<int>((_position.y + 1.0f) * 0.5f * CFG_WINDOW_HEIGHT)
        };
    }

    void setSizePx(vec2<int> size) {
        _size.x = (static_cast<float>(size.x) / CFG_WINDOW_WIDTH) * 2.0f;
        _size.y = (static_cast<float>(size.y) / CFG_WINDOW_HEIGHT) * 2.0f;
    }

    vec2<int> getSizePx() const {
        return {
            static_cast<int>(_size.x * 0.5f * CFG_WINDOW_WIDTH),
            static_cast<int>(_size.y * 0.5f * CFG_WINDOW_HEIGHT)
        };
    }

    void setPositionUS(vec2<float> pos) {
        _position.x = pos.x * 2.0f - 1.0f;
        _position.y = pos.y * 2.0f - 1.0f;
    }

    vec2<float> getPositionUS() const {
        return {
            (_position.x + 1.0f) * 0.5f,
            (_position.y + 1.0f) * 0.5f
        };
    }

    void setSizeUS(vec2<float> size) {
        _size.x = size.x * 2.0f;
        _size.y = size.y * 2.0f;
    }

    vec2<float> getSizeUS() const {
        return {
            _size.x * 0.5f,
            _size.y * 0.5f
        };
    }
};

struct Style {
private:
    Color _primaryColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    Color _secondaryColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    Color _tertiaryColor{ 1.0f, 1.0f, 1.0f, 1.0f };

    bool _border{ false };
    float _borderWidth{ 0.0f };
    float _borderRadius{ 0.0f };
    int _boarderEdges{ 0 };
    Color _borderColor{ 0.0f, 0.0f, 0.0f, 1.0f };

    static Color HexToColor(uint32_t hex) {
        float r = ((hex >> 24) & 0xFF) / 255.0f;
        float g = ((hex >> 16) & 0xFF) / 255.0f;
        float b = ((hex >> 8) & 0xFF) / 255.0f;
        float a = ((hex) & 0xFF) / 255.0f;
        return { r, g, b, a };
    }

    static uint32_t ColorToHex(const Color& c) {
        uint32_t r = static_cast<uint32_t>(c.r * 255.0f) & 0xFF;
        uint32_t g = static_cast<uint32_t>(c.g * 255.0f) & 0xFF;
        uint32_t b = static_cast<uint32_t>(c.b * 255.0f) & 0xFF;
        uint32_t a = static_cast<uint32_t>(c.a * 255.0f) & 0xFF;

        return (r << 24) | (g << 16) | (b << 8) | a;
    }

public:
    void setPrimaryColor(const Color& c) { _primaryColor = c; }
    Color getPrimaryColor() const { return _primaryColor; }

    void setPrimaryColorHex(uint32_t hex) { _primaryColor = HexToColor(hex); }
    uint32_t getPrimaryColorHex() const { return ColorToHex(_primaryColor); }

    void setSecondaryColor(const Color& c) { _secondaryColor = c; }
    Color getSecondaryColor() const { return _secondaryColor; }

    void setSecondaryColorHex(uint32_t hex) { _secondaryColor = HexToColor(hex); }
    uint32_t getSecondaryColorHex() const { return ColorToHex(_secondaryColor); }

    void setTertiaryColor(const Color& c) { _tertiaryColor = c; }
    Color getTertiaryColor() const { return _tertiaryColor; }

    void setTertiaryColorHex(uint32_t hex) { _tertiaryColor = HexToColor(hex); }
    uint32_t getTertiaryColorHex() const { return ColorToHex(_tertiaryColor); }

    void setBorder(bool v) { _border = v; }
    bool getBorder() const { return _border; }

    void setBorderWidth(float v) { _borderWidth = v; }
    float getBorderWidth() const { return _borderWidth; }

    void setBorderRadius(float v) { _borderRadius = v; }
    float getBorderRadius() const { return _borderRadius; }

    void setBorderEdges(int v) { _boarderEdges = v; }
    int getBorderEdges() const { return _boarderEdges; }

    void setBorderColor(const Color& c) { _borderColor = c; }
    Color getBorderColor() const { return _borderColor; }

    void setBorderColorHex(uint32_t hex) { _borderColor = HexToColor(hex); }
    uint32_t getBorderColorHex() const { return ColorToHex(_borderColor); }
};