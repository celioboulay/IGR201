#version 330 core

in vec4 fPosition;
in vec3 fNormal; // receive fNormal from the vertex shader
in vec2 fTexCoord;

//out vec4 FragColor;
out vec4 color; // Shader output: the color response attached to this fragment

uniform vec3 camPos;
uniform vec3 objectColor;
uniform int isSun;
uniform sampler2D backgroundTex;


struct Material {
sampler2D albedoTex; // texture unit, relate to glActivateTexture(GL_TEXTURE0 + i)
};
uniform Material material;


void main() {
    vec3 texColor = texture(material.albedoTex, fTexCoord).rgb;
    vec3 n = normalize(fNormal);
    vec3 lightPos = vec3(0.0, 0.0, 0.0); // Position du Soleil
    vec3 l = normalize(lightPos - fPosition.xyz);

    vec3 v = normalize(camPos - fPosition.xyz);   //view vector
    vec3 r = reflect(-l, n);    //reflection vector

    // Ambient color
    //vec3 ambient = vec3(0.2, 0.2, 0.2);
    //vec3 ambient = 0.2 * objectColor;


    // Diffuse color
    vec3 diffuseColor = vec3(1.0, 1.0, 1.0);
    float diff = max(dot(n, l), 0.0);
    vec3 diffuse = diff * diffuseColor;

    // Specular color
    vec3 specularColor = vec3(1.0, 1.0, 1.0); // Set your specular color here
    float shininess = 32.0; // Shininess factor
    float spec = pow(max(dot(r, v), 0.0), shininess);
    vec3 specular = spec * specularColor;



    // Final color calculation

    if (isSun == 1) { //the sun remains a yellow sphere, whereas the earth and moon uses their texture
        color = vec4(objectColor, 1.0);;
    } else {
        color = vec4(texColor + diffuse + specular, 1.0);;
    }
}
