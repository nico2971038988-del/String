#version 120

uniform sampler2D texture;

uniform vec2 beamCenter;
uniform float beamAngle;
uniform float beamWidth;
uniform float edgeSoftness;
uniform float intensity;
uniform float beatPulse;
uniform vec2 resolution;

uniform vec3 shadowTint;
uniform vec3 lightTint;
uniform float shadowExposure;
uniform float lightExposure;
uniform float saturation;

float luminance(vec3 color)
{
    return dot(
        color,
        vec3(0.299, 0.587, 0.114)
    );
}

vec3 changeSaturation(
    vec3 color,
    float amount)
{
    float gray = luminance(color);

    return mix(
        vec3(gray),
        color,
        amount
    );
}

void main()
{
    // gl_TexCoord 只用于读取当前精灵纹理
    vec2 textureUV =
        gl_TexCoord[0].xy;

    vec4 source =
        texture2D(texture, textureUV);

    // gl_FragCoord 用于计算整个窗口中的光带位置
    vec2 screenUV =
        gl_FragCoord.xy /
        max(resolution, vec2(1.0));

    // OpenGL 屏幕坐标原点位于左下角，转换为左上角
    screenUV.y = 1.0 - screenUV.y;

    vec2 position =
        screenUV - beamCenter;

    position.x *=
        resolution.x /
        max(resolution.y, 1.0);

    vec2 beamDirection = vec2(
        cos(beamAngle),
        sin(beamAngle)
    );

    vec2 beamNormal = vec2(
        -beamDirection.y,
        beamDirection.x
    );

    float distanceFromAxis =
        abs(dot(position, beamNormal));

    float halfWidth =
        beamWidth * 0.5;

    float beamMask =
        1.0 -
        smoothstep(
            halfWidth - edgeSoftness,
            halfWidth + edgeSoftness,
            distanceFromAxis
        );

    float coreMask =
        1.0 -
        smoothstep(
            halfWidth * 0.15,
            halfWidth * 0.55,
            distanceFromAxis
        );

    vec3 original =
        changeSaturation(
            source.rgb,
            saturation
        );

    // 光带外部压暗并染成暗红色
    vec3 shadowColor =
        mix(
            original * 0.15,
            shadowTint,
            0.65
        );

    shadowColor *=
        max(shadowExposure, 0.05);

    // 光带内部恢复纹理，并染成金色
    float pulseGain =
        1.0 + beatPulse * 0.65;

    vec3 lightColor =
        original *
        lightTint *
        lightExposure *
        pulseGain;

    vec3 coreColor =
        vec3(1.0, 0.92, 0.70) *
        (0.7 + luminance(original));

    lightColor = mix(
        lightColor,
        coreColor,
        coreMask * 0.005
    );

    vec3 stageColor = mix(
        shadowColor,
        lightColor,
        beamMask
    );

    vec3 finalColor = mix(
        source.rgb,
        stageColor,
        clamp(intensity, 0.0, 1.0)
    );

    gl_FragColor = vec4(
        finalColor,
        source.a
    );
}