export * from "./types";

import type { BezierCurve as _BezierCurve, Vec3f } from "./types";
export type BezierCurve = _BezierCurve & { color: Vec3f };

import type { MainModule } from "./types";
export type VektorModule = MainModule;
