#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 pieceColor;

out vec4 FragColor;

void main() {
    // Paramètres de lumière
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    // Composante ambiante
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * pieceColor;
    
    // Composante diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * pieceColor;
    
    // Résultat final
    vec3 result = ambient + diffuse;
    
    FragColor = vec4(result, 1.0);
}
