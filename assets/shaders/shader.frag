#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    // For now, just output a solid color since we don't have textures loaded yet
    // Once textures are implemented, use: FragColor = texture(uTexture, vTexCoord);
    FragColor = vec4(0.2, 0.6, 1.0, 1.0); // Nice blue color
}
