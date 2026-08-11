#version 120

uniform sampler2D texture;

// Coordinates are normalized to the rendered texture: (0,0) top-left, (1,1) bottom-right.
uniform vec2  beamCenter;
uniform float beamAngle;       // radians; direction along the beam
uniform float beamWidth;       // full width, recommended 0.12 - 0.28
uniform float edgeSoftness;    // recommended 0.008 - 0.035
uniform float intensity;       // 0 = no effect, 1 = full stage lighting
uniform float beatPulse;       // 0 - 1, drive this from music accents
uniform vec2  resolution;

uniform vec3 shadowTint;       // recommended vec3(0.055, 0.018, 0.022)
uniform vec3 lightTint;        // recommended vec3(1.00, 0.72, 0.28)
uniform float shadowExposure;  // recommended 0.14
uniform float lightExposure;   // recommended 1.35
uniform float saturation;      // recommended 1.10

float luminance(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

vec3 changeSaturation(vec3 c, float amount)
{
    return mix(vec3(luminance(c)), c, amount);
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 source = texture2D(texture, uv);

    // Keep diagonal beam width visually stable on non-square screens.
    vec2 p = uv - beamCenter;
    p.x *= resolution.x / max(resolution.y, 1.0);

    vec2 beamDirection = vec2(cos(beamAngle), sin(beamAngle));
    vec2 beamNormal = vec2(-beamDirection.y, beamDirection.x);
    float distanceFromAxis = abs(dot(p, beamNormal));

    float halfWidth = beamWidth * 0.5;
    float beamMask = 1.0 - smoothstep(
        halfWidth - edgeSoftness,
        halfWidth + edgeSoftness,
        distanceFromAxis
    );

    // A narrow, brighter core gives the cut light a graphic gold-white accent.
    float coreMask = 1.0 - smoothstep(
        halfWidth * 0.18,
        halfWidth * 0.60,
        distanceFromAxis
    );

    vec3 original = changeSaturation(source.rgb, saturation);
    float shape = smoothstep(0.08, 0.70, luminance(original));

    // Outside the beam, preserve only a faint red-brown silhouette.
    vec3 shadow = mix(shadowTint * 0.45, original * shadowTint * 2.0, shape);
    shadow *= shadowExposure;

    // Inside the beam, restore texture and push highlights toward gold-white.
    float pulseGain = 1.0 + beatPulse * 0.38;
    vec3 lit = original * lightTint * lightExposure * pulseGain;
    vec3 goldWhite = vec3(1.0, 0.91, 0.70) * (0.75 + luminance(original));
    lit = mix(lit, goldWhite, coreMask * (0.30 + 0.30 * beatPulse));

    vec3 stageColor = mix(shadow, lit, beamMask);
    vec3 result = mix(source.rgb, stageColor, clamp(intensity, 0.0, 1.0));

    // Preserve PNG transparency exactly.
    gl_FragColor = vec4(result, source.a);
}
