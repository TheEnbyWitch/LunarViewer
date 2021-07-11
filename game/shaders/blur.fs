#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

// NOTE: Render size values must be passed from code
const float renderWidth = 320;
const float renderHeight = 240;

float offset[3] = float[](0.0, 1, 2);
float weight[3] = float[](0.2270270270, 0.25, 0.25);

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord * vec2(1,-1), 0.0).rgba*weight[0];
    
    for (int i = 1; i < 3; i++) 
    {
        texelColor += texture(texture0, (fragTexCoord * vec2(1,-1)) + vec2(0.0, offset[i])/(renderWidth / 1), 0.0).rgba*weight[i];
        texelColor += texture(texture0, (fragTexCoord * vec2(1,-1)) - vec2(0.0, offset[i])/(renderWidth / 1), 0.0).rgba*weight[i];
    }

    finalColor = vec4(texelColor);
}