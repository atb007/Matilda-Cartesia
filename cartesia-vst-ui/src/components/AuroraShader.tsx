import { useEffect, useRef, useState } from "react";
import { AURORA } from "../auroraConfig";

function buildFragmentShader() {
  const [gr, gg, gb] = AURORA.green;
  const [tr, tg, tb] = AURORA.teal;
  const [vr, vg, vb] = AURORA.violet;
  const [mr, mg, mb] = AURORA.magenta;

  return `
precision highp float;
uniform vec2 uResolution;
uniform float uTime;
out vec4 outColor;

float hash(vec2 p) {
  return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  vec2 u = f * f * (3.0 - 2.0 * f);
  return mix(
    mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x),
    mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x),
    u.y
  );
}

float fbm(vec2 p) {
  float v = 0.0;
  float a = 0.5;
  mat2 rot = mat2(0.87758, 0.47943, -0.47943, 0.87758);
  for (int i = 0; i < 5; i++) {
    v += a * noise(p);
    p = rot * p * 2.02 + vec2(1.7, 9.2);
    a *= 0.5;
  }
  return v;
}

void main() {
  vec2 uv = vec2(gl_FragCoord.x / uResolution.x, 1.0 - gl_FragCoord.y / uResolution.y);
  float skyY = 1.0 - uv.y;
  vec2 p = vec2(uv.x * ${AURORA.noiseScaleX}, skyY * ${AURORA.noiseScaleY} + 0.08 + uTime * ${AURORA.drift});

  float wave = sin(uv.x * 5.5 + uTime * 0.4) * ${AURORA.waveAmp1}
             + sin(uv.x * 9.0 - uTime * 0.25) * ${AURORA.waveAmp2};
  float curtain = smoothstep(${AURORA.curtainTop}, ${AURORA.curtainLow}, skyY + wave);
  float bands = fbm(p + vec2(fbm(p + uTime * 0.1) * 1.8, 0.0));
  float shimmer = fbm(vec2(uv.x * 3.5, skyY * 2.1 - uTime * 0.15));
  float rays = pow(max(0.0, sin(uv.x * 12.0 + uTime * 0.2) * 0.5 + 0.5), 2.8) * ${AURORA.rays};

  float intensity = curtain * (bands * ${AURORA.bands} + shimmer * ${AURORA.shimmer} + rays);
  intensity = pow(clamp(intensity, 0.0, 1.0), ${AURORA.intensityPow});

  float skyFade = 1.0 - smoothstep(${AURORA.skyFadeFrom}, ${AURORA.skyFadeTo}, uv.y);
  intensity *= skyFade;

  vec3 green = vec3(${gr}, ${gg}, ${gb});
  vec3 teal  = vec3(${tr}, ${tg}, ${tb});
  vec3 violet = vec3(${vr}, ${vg}, ${vb});
  vec3 magenta = vec3(${mr}, ${mg}, ${mb});

  float hue = sin(uv.x * 3.1415 + uTime * 0.18) * 0.5 + 0.5;
  vec3 col = mix(mix(green, teal, hue), mix(violet, magenta, hue * 0.65), uv.x * 0.6);
  col *= intensity * ${AURORA.colorGain};

  float alpha = clamp(intensity * ${AURORA.alphaGain}, 0.0, 0.9);
  outColor = vec4(col * alpha, alpha);
}
`;
}

const FRAGMENT_GLSL1 = buildFragmentShader().replace("out vec4 outColor;", "").replace(
  "outColor = ",
  "gl_FragColor = ",
);

const VERTEX_GLSL1 = `
attribute vec2 aPos;
void main() {
  gl_Position = vec4(aPos, 0.0, 1.0);
}
`;

type GlContext = WebGLRenderingContext | WebGL2RenderingContext;

function compileShader(gl: GlContext, type: number, source: string) {
  const shader = gl.createShader(type);
  if (!shader) return null;
  gl.shaderSource(shader, source);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    console.warn("AuroraShader compile:", gl.getShaderInfoLog(shader));
    gl.deleteShader(shader);
    return null;
  }
  return shader;
}

type Props = {
  className?: string;
  style?: React.CSSProperties;
};

/**
 * Animated aurora curtains — WebGL overlay above the hero raster bg.
 * Tune via `src/auroraConfig.ts`.
 */
export function AuroraShader({ className, style }: Props) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const rafRef = useRef<number>(0);
  const [useFallback, setUseFallback] = useState(false);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const gl2 = canvas.getContext("webgl2", {
      alpha: true,
      premultipliedAlpha: true,
      antialias: false,
    });

    const isWebGL2 = !!gl2;
    const gl = gl2 ?? canvas.getContext("webgl", {
      alpha: true,
      premultipliedAlpha: true,
      antialias: false,
    });

    if (!gl) {
      setUseFallback(true);
      return;
    }

    const fragmentSource = isWebGL2
      ? buildFragmentShader()
      : FRAGMENT_GLSL1;

    const vs = compileShader(gl, gl.VERTEX_SHADER, VERTEX_GLSL1);
    const fs = compileShader(gl, gl.FRAGMENT_SHADER, fragmentSource);
    if (!vs || !fs) {
      setUseFallback(true);
      return;
    }

    const program = gl.createProgram();
    if (!program) {
      setUseFallback(true);
      return;
    }
    gl.attachShader(program, vs);
    gl.attachShader(program, fs);
    gl.linkProgram(program);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      console.warn("AuroraShader link:", gl.getProgramInfoLog(program));
      setUseFallback(true);
      return;
    }

    const buf = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buf);
    gl.bufferData(
      gl.ARRAY_BUFFER,
      new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]),
      gl.STATIC_DRAW,
    );

    const aPos = gl.getAttribLocation(program, "aPos");
    const uResolution = gl.getUniformLocation(program, "uResolution");
    const uTime = gl.getUniformLocation(program, "uTime");

    gl.useProgram(program);
    gl.enableVertexAttribArray(aPos);
    gl.vertexAttribPointer(aPos, 2, gl.FLOAT, false, 0, 0);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

    let w = 0;
    let h = 0;

    const resize = () => {
      const parent = canvas.parentElement;
      if (!parent) return;
      const dpr = Math.min(window.devicePixelRatio || 1, 2);
      const nextW = parent.clientWidth;
      const nextH = parent.clientHeight;
      if (nextW < 2 || nextH < 2) return;
      w = Math.max(1, Math.floor(nextW * dpr));
      h = Math.max(1, Math.floor(nextH * dpr));
      canvas.width = w;
      canvas.height = h;
      canvas.style.width = `${nextW}px`;
      canvas.style.height = `${nextH}px`;
      gl.viewport(0, 0, w, h);
    };

    resize();
    const ro = new ResizeObserver(() => resize());
    ro.observe(canvas.parentElement ?? canvas);

    const start = performance.now();
    const draw = (now: number) => {
      if (w < 2 || h < 2) {
        resize();
        rafRef.current = requestAnimationFrame(draw);
        return;
      }
      gl.clearColor(0, 0, 0, 0);
      gl.clear(gl.COLOR_BUFFER_BIT);
      gl.uniform2f(uResolution, w, h);
      gl.uniform1f(uTime, (now - start) * 0.001);
      gl.drawArrays(gl.TRIANGLES, 0, 6);
      rafRef.current = requestAnimationFrame(draw);
    };
    rafRef.current = requestAnimationFrame(draw);

    return () => {
      cancelAnimationFrame(rafRef.current);
      ro.disconnect();
      gl.deleteProgram(program);
      gl.deleteShader(vs);
      gl.deleteShader(fs);
      gl.deleteBuffer(buf);
    };
  }, []);

  if (useFallback) {
    return (
      <div
        className={className}
        aria-hidden
        style={{
          position: "absolute",
          inset: 0,
          pointerEvents: "none",
          background: `
            radial-gradient(ellipse 90% 55% at 30% 10%, rgba(45, 230, 145, 0.48), transparent 72%),
            radial-gradient(ellipse 75% 48% at 62% 8%, rgba(170, 65, 240, 0.4), transparent 68%),
            radial-gradient(ellipse 100% 42% at 45% 16%, rgba(60, 200, 185, 0.3), transparent 62%)
          `,
          WebkitMaskImage: AURORA.maskGradient,
          maskImage: AURORA.maskGradient,
          animation: "aurora-drift 12s ease-in-out infinite alternate",
          ...style,
        }}
      />
    );
  }

  return (
    <canvas
      ref={canvasRef}
      className={className}
      aria-hidden
      style={{
        position: "absolute",
        inset: 0,
        width: "100%",
        height: "100%",
        pointerEvents: "none",
        ...style,
      }}
    />
  );
}
