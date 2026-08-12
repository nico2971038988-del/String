uniform sampler2D texture;
uniform sampler2D outlineTexture;
uniform float amount;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec4 normal = texture2D(texture, uv);
    vec4 line = texture2D(outlineTexture, uv);
    vec3 outlineWorld = line.rgb * line.a;

    // The background fades to black while the white contour fades in.
    vec3 result = mix(normal.rgb, outlineWorld, amount);
    gl_FragColor = vec4(result, 1.0);
}

