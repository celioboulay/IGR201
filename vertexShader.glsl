#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal; // def the attribute vNormal for normals
layout(location = 2) in vec2 vTexCord;


uniform mat4 viewMat, projMat, modelMat, normalMat;


out vec3 fNormal; // transmit fNormal to the fragment shader
out vec4 fPosition;
out vec2 fTexCoord;

void main() {
    gl_Position = projMat * viewMat * modelMat * vec4(vPosition, 1.0);
    fNormal = mat3(normalMat) * vNormal; // Pass the vertex normal to the fragment shader
    fPosition = gl_Position;
    fTexCoord = vTexCord;
}
