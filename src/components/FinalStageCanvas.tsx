import React, { useEffect, useRef } from "react";
import { Paper } from "@mui/material";
import type { Vec2f, BezierCurve, Vec3f } from "@/vektor";

type Bounds = {
  left: number;
  right: number;
  bottom: number;
  top: number;
};

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
  const ratio = aspectRatio ?? width / height;

  if (height > width) {
    bounds.bottom -= padding * height;
    bounds.top += padding * height;

    const newWidth = height * (2 * padding + 1) * ratio;
    bounds.left = (width - newWidth) / 2;
    bounds.right = (width + newWidth) / 2;
  } else {
    bounds.left -= padding * width;
    bounds.right += padding * width;

    const newHeight = (width * (2 * padding + 1)) / ratio;
    bounds.top = (newHeight - height) / 2;
    bounds.bottom = -(height + newHeight) / 2;
  }
}

function plotCurves(
  calculator: Desmos.Calculator,
  curves: BezierCurve[],
  desmosColor: "solid" | "auto",
  invertedColors: boolean
) {
  const INF = 1e6;
  const bounds: Bounds = { left: INF, right: -INF, top: -INF, bottom: INF };
  const updateBounds = (p: Vec2f) => {
    bounds.left = Math.min(bounds.left, p.x);
    bounds.right = Math.max(bounds.right, p.x);
    bounds.top = Math.max(bounds.top, -p.y);
    bounds.bottom = Math.min(bounds.bottom, -p.y);
  };

  for (const curve of curves) {
    const { p0, p1, p2, p3 } = curve;
    [p0, p1, p2, p3].forEach((p) => updateBounds(p));

    const latex = `(
          (${p0.x}*(1-t)^3 + 3*${p1.x}*(1-t)^2*t + 3*${p2.x}*(1-t)*t^2 + ${p3.x}*t^3),
          -(${p0.y}*(1-t)^3 + 3*${p1.y}*(1-t)^2*t + 3*${p2.y}*(1-t)*t^2 + ${p3.y}*t^3)
        )`;

    const toByte = (x: number) =>
      Math.max(
        0,
        Math.min(255, Math.round((invertedColors ? 1.0 - x : x) * 255))
      );

    const { r, g, b } =
      desmosColor === "solid" ? { r: 0.78, g: 0.26, b: 0.25 } : curve.color;
    const rr = toByte(r).toString(16).padStart(2, "0");
    const gg = toByte(g).toString(16).padStart(2, "0");
    const bb = toByte(b).toString(16).padStart(2, "0");

    calculator.setExpression({
      latex,
      color: `#${rr}${gg}${bb}`.toUpperCase(),
    });
  }

  const currBounds = calculator.graphpaperBounds.mathCoordinates;
  adjustBoundsForAspect({
    bounds,
    aspectRatio: currBounds.width / currBounds.height,
  });
  calculator.setMathBounds(bounds);
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
  const paperRef = useRef<HTMLDivElement | null>(null);
  const calculatorRef = useRef<Desmos.Calculator | null>(null);

  useEffect(() => {
    if (!window.Desmos || !paperRef.current) return;

    calculatorRef.current = window.Desmos.GraphingCalculator(paperRef.current, {
      keypad: false,
      expressions: false,
      settingsMenu: false,
      zoomButtons: false,
      lockViewport: true,
      invertedColors,
    });

    calculatorRef.current.updateSettings({
      showGrid: false,
      showXAxis: false,
      showYAxis: false,
    });

    if (curves) {
      plotCurves(calculatorRef.current, curves, desmosColor, invertedColors);
    }

    return () => calculatorRef.current?.destroy();
  });

  return (
    <Paper
      ref={paperRef}
      elevation={3}
      sx={{ height: "100%", display: "grid", placeItems: "center" }}
    ></Paper>
  );
}
