#version 330 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aTranslation;
layout (location = 3) in vec2 aScale;

uniform mat4 uProjection;

out vec2 vTexCoord;

void main() {
    vec3 worldPos = vec3(aScale * aPosition, 0.0) + aTranslation;
    gl_Position = uProjection * vec4(worldPos, 1.0);
    vTexCoord = aTexCoord;
}
