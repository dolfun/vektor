import React, { useEffect, useRef, useState } from "react";
import { Paper } from "@mui/material";
import type { Vec2f, BezierCurve } from "@/vektor";

type Bounds = { left: number; right: number; bottom: number; top: number };

const BATCH_SIZE = 20;

function adjustBoundsForAspect({
  bounds,
  padding = 0,
  aspectRatio,
}: {
  bounds: Bounds;
  padding?: number;
  aspectRatio?: number;
}) {
  const width = bounds.right - bounds.left;
  const height = bounds.top - bounds.bottom;
  const targetRatio = aspectRatio ?? width / height;

  const paddedWidth = width * (1 + 2 * padding);
  const paddedHeight = height * (1 + 2 * padding);

  const paddedRatio = paddedWidth / paddedHeight;
  let finalWidth = paddedWidth;
  let finalHeight = paddedHeight;

  if (paddedRatio < targetRatio) {
    finalWidth = targetRatio * paddedHeight;
  } else if (paddedRatio > targetRatio) {
    finalHeight = paddedWidth / targetRatio;
  }

  const cx = (bounds.left + bounds.right) / 2;
  const cy = (bounds.top + bounds.bottom) / 2;

  bounds.left = cx - finalWidth / 2;
  bounds.right = cx + finalWidth / 2;
  bounds.bottom = cy - finalHeight / 2;
  bounds.top = cy + finalHeight / 2;
}

function toHexByte(n: number, inverted: boolean) {
  const val = Math.max(
    0,
    Math.min(255, Math.round((inverted ? 1 - n : n) * 255))
  );
  return val.toString(16).padStart(2, "0");
}

function makeLatex(curve: BezierCurve) {
  const { p0, p1, p2, p3 } = curve;
  return `(
    (${p0.x}*(1-t)^3 + 3*${p1.x}*(1-t)^2*t + 3*${p2.x}*(1-t)*t^2 + ${p3.x}*t^3),
    -(${p0.y}*(1-t)^3 + 3*${p1.y}*(1-t)^2*t + 3*${p2.y}*(1-t)*t^2 + ${p3.y}*t^3)
  )`;
}

function updateBoundsPoint(bounds: Bounds, p: Vec2f) {
  bounds.left = Math.min(bounds.left, p.x);
  bounds.right = Math.max(bounds.right, p.x);
  bounds.top = Math.max(bounds.top, -p.y);
  bounds.bottom = Math.min(bounds.bottom, -p.y);
}

function computeBounds(curves: BezierCurve[]): Bounds {
  const MAX_BOUND = 1e6;

  const b: Bounds = {
    left: MAX_BOUND,
    right: -MAX_BOUND,
    top: -MAX_BOUND,
    bottom: MAX_BOUND,
  };
  for (const c of curves) {
    updateBoundsPoint(b, c.p0);
    updateBoundsPoint(b, c.p1);
    updateBoundsPoint(b, c.p2);
    updateBoundsPoint(b, c.p3);
  }
  return b;
}

const nextIdle = () =>
  new Promise<void>((resolve) => {
    if ("requestIdleCallback" in window) {
      (window as any).requestIdleCallback(resolve, { timeout: 50 });
    } else {
      requestAnimationFrame(() => resolve());
    }
  });

async function streamCurves({
  calculator,
  curves,
  desmosColor,
  invertedColors,
  cancelSignal,
  onProgress,
}: {
  calculator: Desmos.Calculator;
  curves: BezierCurve[];
  desmosColor: "solid" | "auto";
  invertedColors: boolean;
  cancelSignal: { cancelled: boolean };
  onProgress: (p: number) => void;
}) {
  let batch: Desmos.ExpressionState[] = [];
  for (let i = 0; i < curves.length; i++) {
    if (cancelSignal.cancelled) break;

    const c = curves[i];
    const { r, g, b } =
      desmosColor === "solid" ? { r: 0.78, g: 0.26, b: 0.25 } : c.color;
    const color = `#${toHexByte(r, invertedColors)}${toHexByte(
      g,
      invertedColors
    )}${toHexByte(b, invertedColors)}`.toUpperCase();

    batch.push({ id: `c_${i}`, latex: makeLatex(c), color });

    const isBatchEnd = (i + 1) % BATCH_SIZE === 0 || i === curves.length - 1;
    if (isBatchEnd) {
      (calculator as any).setExpressions(batch);
      batch = [];
      onProgress((i + 1) / curves.length);
      await nextIdle();
    }
  }
}

export function FinalStageCanvas({
  curves,
  desmosColor,
  invertedColors,
}: {
  curves: BezierCurve[] | null;
  desmosColor: "solid" | "auto";
  invertedColors: boolean;
}) {
  const graphRef = useRef<HTMLDivElement | null>(null);
  const calculatorRef = useRef<Desmos.Calculator | null>(null);
  const cancelRef = useRef<{ cancelled: boolean }>({ cancelled: false });
  const [progress, setProgress] = useState(0);
  const [showBar, setShowBar] = useState(false);

  useEffect(() => {
    if (!window.Desmos || !graphRef.current) return;
    const calc = window.Desmos.GraphingCalculator(graphRef.current, {
      keypad: false,
      expressions: false,
      settingsMenu: false,
      zoomButtons: false,
      lockViewport: true,
      invertedColors,
    });
    calc.updateSettings({
      showGrid: false,
      showXAxis: false,
      showYAxis: false,
    });
    calculatorRef.current = calc;
    return () => calc.destroy();
  }, []);

  useEffect(() => {
    calculatorRef.current?.updateSettings({ invertedColors });
  }, [invertedColors]);

  useEffect(() => {
    const calc = calculatorRef.current;
    if (!calc) return;

    cancelRef.current.cancelled = true;

    Promise.resolve().then(async () => {
      setProgress(0);
      setShowBar(!!curves?.length);
      cancelRef.current = { cancelled: false };
      if (!curves?.length) return;

      const rawBounds = computeBounds(curves);
      const { width, height } = calc.graphpaperBounds.mathCoordinates;
      const aspect = width / height;
      const b = { ...rawBounds };
      adjustBoundsForAspect({ bounds: b, aspectRatio: aspect });
      calc.setMathBounds(b);

      await streamCurves({
        calculator: calc,
        curves,
        desmosColor,
        invertedColors,
        cancelSignal: cancelRef.current,
        onProgress: setProgress,
      });

      if (!cancelRef.current.cancelled) {
        setProgress(1);
        setShowBar(false);
      }
    });

    return () => {
      cancelRef.current.cancelled = true;
    };
  }, [curves, desmosColor, invertedColors]);

  return (
    <Paper
      elevation={3}
      sx={{ height: "100%", display: "grid", placeItems: "center" }}
    >
      <div style={{ position: "relative", width: "100%", height: "100%" }}>
        <div ref={graphRef} style={{ position: "absolute", inset: 0 }} />
        {showBar && (
          <div
            style={{
              position: "absolute",
              left: 0,
              right: 0,
              bottom: 0,
              height: 2,
              background: "#ddd",
            }}
          >
            <div
              style={{
                height: "100%",
                width: `${Math.round(progress * 100)}%`,
                background: "#00C853",
              }}
            />
          </div>
        )}
      </div>
    </Paper>
  );
}
