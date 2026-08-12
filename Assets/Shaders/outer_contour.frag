uniform sampler2D texture;
uniform vec2 texelSize;
uniform float lineWidth;

float alphaAt(vec2 uv) {
    return texture2D(texture, uv).a;
}

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec2 dx = vec2(texelSize.x * lineWidth, 0.0);
    vec2 dy = vec2(0.0, texelSize.y * lineWidth);

    float center = alphaAt(uv);
    float around = max(max(alphaAt(uv + dx), alphaAt(uv - dx)),
                       max(alphaAt(uv + dy), alphaAt(uv - dy)));
    around = max(around, max(max(alphaAt(uv + dx + dy), alphaAt(uv - dx + dy)),
                             max(alphaAt(uv + dx - dy), alphaAt(uv - dx - dy))));

    // Outer contour only: transparent pixels next to opaque silhouette pixels.
    float line = smoothstep(0.05, 0.6, around) * (1.0 - smoothstep(0.05, 0.6, center));
    gl_FragColor = vec4(vec3(1.0), line);
}

