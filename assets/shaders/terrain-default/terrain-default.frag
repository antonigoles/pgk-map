#version 460 core

in vec3 FragPos;
in vec3 VecPos;
// in vec3 color;
// in vec3 Normal;

out vec4 FragColor;

void main()
{
    float actualSize = 100000.0;
    float earthRadius = 6371000.0;

    float scale = actualSize / earthRadius;

    float ht = length(FragPos) - actualSize;

    vec3 color;

    if      (ht < 0  )   color = vec3(0.,       0.,        1.); //blue
    else if (ht < scale * 500)   color = vec3(0.,       ht/(scale*500),    pow(1.0 - ht/(scale*500), 8)); //->green
    else if (ht < scale * 1000)  color = vec3(ht/(scale*500)-1, 1.,        0.); //->yellow
    else if (ht < scale * 2000)  color = vec3(1.,       2.-ht/(scale*1000),0.); //->red
    else                 color = vec3(1.,       ht/(scale*2000)-1 ,ht/(scale*2000)-1);  //->white

    FragColor = vec4(color, 1.0);

    // if (abs(FragPos.x-1) < 0.2 || abs(FragPos.z-1) < 0.2) {
    //     FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    // } else {
    //     FragColor = vec4(0.1, 0.9, FragPos.y / 30, 1.0);
    // }
    // FragColor = vec4(0.5, 1.0, 0.2, 1.0);
}
