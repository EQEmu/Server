struct vec2
{
    float x, y;
    static length_t length();

    vec2();
    vec2(vec2 const &v);
    vec2(float scalar);
    vec2(float s1, float s2);
    vec2(glm::vec3 const &v);
    vec2(glm::vec4 const &v);

    vec2 &operator=(vec2 const &v);
};

vec2 operator+(vec2 const &v, float scalar);
vec2 operator+(float scalar, vec2 const &v);
vec2 operator+(vec2 const &v1, vec2 const &v2);
vec2 operator-(vec2 const &v, float scalar);
vec2 operator-(float scalar, vec2 const &v);
vec2 operator-(vec2 const &v1, vec2 const &v2);
vec2 operator*(vec2 const &v, float scalar);
vec2 operator*(float scalar, vec2 const &v);
vec2 operator*(vec2 const &v1, vec2 const &v2);
vec2 operator/(vec2 const &v, float scalar);
vec2 operator/(float scalar, vec2 const &v);
vec2 operator/(vec2 const &v1, vec2 const &v2);
vec2 operator%(vec2 const &v, float scalar);
vec2 operator%(float scalar, vec2 const &v);
vec2 operator%(vec2 const &v1, vec2 const &v2);
bool operator==(vec2 const &v1, vec2 const &v2);
bool operator!=(vec2 const &v1, vec2 const &v2);

struct vec3
{
    float x, y, z;

    static length_t length();

    vec3();
    vec3(vec3 const &v);
    vec3(float scalar);
    vec3(float s1, float s2, float s3);
    vec3(glm::vec2 const &a, float b);
    vec3(float a, glm::vec2 const &b);
    vec3(glm::vec4 const &v);

    vec3 &operator=(vec3 const &v);
};

vec3 operator+(vec3 const &v, float scalar);
vec3 operator+(float scalar, vec3 const &v);
vec3 operator+(vec3 const &v1, vec3 const &v2);
vec3 operator-(vec3 const &v, float scalar);
vec3 operator-(float scalar, vec3 const &v);
vec3 operator-(vec3 const &v1, vec3 const &v2);
vec3 operator*(vec3 const &v, float scalar);
vec3 operator*(float scalar, vec3 const &v);
vec3 operator*(vec3 const &v1, vec3 const &v2);
vec3 operator/(vec3 const &v, float scalar);
vec3 operator/(float scalar, vec3 const &v);
vec3 operator/(vec3 const &v1, vec3 const &v2);
vec3 operator%(vec3 const &v, float scalar);
vec3 operator%(float scalar, vec3 const &v);
vec3 operator%(vec3 const &v1, vec3 const &v2);
bool operator==(vec3 const &v1, vec3 const &v2);
bool operator!=(vec3 const &v1, vec3 const &v2);


struct vec4
{

    float x, y, z, w;

    static length_t length();

    vec4();
    vec4(vec4 const &v);
    vec4(float scalar);
    vec4(float s1, float s2, float s3, float s4);
    vec4(vec2 const &a, vec2 const &b);
    vec4(vec2 const &a, float b, float c);
    vec4(float a, vec2 const &b, float c);
    vec4(float a, float b, vec2 const &c);
    vec4(vec3 const &a, float b);
    vec4(float a, vec3 const &b);

    vec4 &operator=(vec4 const &v);
};

vec4 operator+(vec4 const &v, float scalar);
vec4 operator+(float scalar, vec4 const &v);
vec4 operator+(vec4 const &v1, vec4 const &v2);
vec4 operator-(vec4 const &v, float scalar);
vec4 operator-(float scalar, vec4 const &v);
vec4 operator-(vec4 const &v1, vec4 const &v2);
vec4 operator*(vec4 const &v, float scalar);
vec4 operator*(float scalar, vec4 const &v);
vec4 operator*(vec4 const &v1, vec4 const &v2);
vec4 operator/(vec4 const &v, float scalar);
vec4 operator/(float scalar, vec4 const &v);
vec4 operator/(vec4 const &v1, vec4 const &v2);
vec4 operator%(vec4 const &v, float scalar);
vec4 operator%(float scalar, vec4 const &v);
vec4 operator%(vec4 const &v1, vec4 const &v2);
bool operator==(vec4 const &v1, vec4 const &v2);
bool operator!=(vec4 const &v1, vec4 const &v2);
