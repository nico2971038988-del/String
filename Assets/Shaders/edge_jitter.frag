#version 120

uniform sampler2D texture;
uniform float time;
uniform vec2 resolution;
uniform float strength;
uniform float speed;
uniform float chromaticOffset;
uniform float burstAmount;
uniform float enabled;

float hash(float n)
{
    return fract(sin(n * 91.3458) * 47453.5453);
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    if (enabled < 0.5)
    {
        gl_FragColor = texture2D(texture, uv) * gl_Color;
        return;
    }

    // Horizontal strips update at discrete times, creating unstable edges.
    float bandCount = 160.0;
    float band = floor(uv.y * bandCount);
    float tick = floor(time * 20.0 * max(speed, 0.01));
    float noiseValue = hash(band + tick * 17.0);

    float smallJitter = (noiseValue - 0.5) * 2.2 * strength;
    float burstMask = step(0.92, noiseValue);
    float largeJitter = (noiseValue - 0.5) * 14.0
                      * burstMask * strength * burstAmount;

    float wave = (
        sin(uv.y * 180.0 + time * 13.0 * speed) * 0.8 +
        sin(uv.y * 43.0  - time * 7.0  * speed) * 0.5
    ) * strength;

    float offsetX = (smallJitter + largeJitter + wave)
                  / max(resolution.x, 1.0);
    vec2 distortedUV = uv + vec2(offsetX, 0.0);

    float colorShiftPixels = chromaticOffset
                           + burstMask * 3.0 * burstAmount;
    float colorShiftX = colorShiftPixels / max(resolution.x, 1.0);

    vec4 center = texture2D(texture, distortedUV);
    vec4 cyanSample = texture2D(
        texture, distortedUV - vec2(colorShiftX, 0.0));
    vec4 magentaSample = texture2D(
        texture, distortedUV + vec2(colorShiftX, 0.0));

    vec3 color = vec3(
        magentaSample.r,
        cyanSample.g,
        cyanSample.b
    );

    // Alpha follows every displaced sample, preserving a genuinely
    // transparent background around the PNG line art.
    float alpha = max(center.a, max(cyanSample.a, magentaSample.a));
    gl_FragColor = vec4(color, alpha) * gl_Color;
}
