// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
}

export interface ClassHandle {
  isAliasOf(other: ClassHandle): boolean;
  delete(): void;
  deleteLater(): this;
  isDeleted(): boolean;
  // @ts-ignore - If targeting lower than ESNext, this symbol might not exist.
  [Symbol.dispose](): void;
  clone(): this;
}
export interface BezierCurveArray extends ClassHandle {
  size(): number;
  get(_0: number): BezierCurve | undefined;
  push_back(_0: BezierCurve): void;
  resize(_0: number, _1: BezierCurve): void;
  set(_0: number, _1: BezierCurve): boolean;
}

export interface ColorImage extends ClassHandle {
  readonly width: number;
  readonly height: number;
  getPixel(_0: number, _1: number): Vec3f;
  setPixel(_0: number, _1: number, _2: Vec3f): void;
}

export interface GreyscaleImage extends ClassHandle {
  readonly width: number;
  readonly height: number;
  getPixel(_0: number, _1: number): number;
  setPixel(_0: number, _1: number, _2: number): void;
}

export interface BinaryImage extends ClassHandle {
  readonly width: number;
  readonly height: number;
  getPixel(_0: number, _1: number): number;
  setPixel(_0: number, _1: number, _2: number): void;
}

export type Vec3f = {
  r: number,
  g: number,
  b: number
};

export type Vec2f = {
  x: number,
  y: number
};

export type BezierCurve = {
  p0: Vec2f,
  p1: Vec2f,
  p2: Vec2f,
  p3: Vec2f
};

interface EmbindModule {
  BezierCurveArray: {
    new(): BezierCurveArray;
  };
  ColorImage: {
    new(_0: number, _1: number): ColorImage;
  };
  GreyscaleImage: {
    new(_0: number, _1: number): GreyscaleImage;
  };
  BinaryImage: {
    new(_0: number, _1: number): BinaryImage;
  };
  traceEdges(_0: BinaryImage): BezierCurveArray;
  renderCurves(_0: number, _1: number, _2: BezierCurveArray): GreyscaleImage;
  detectEdges(_0: ColorImage, _1: number): BinaryImage;
}

export type MainModule = WasmModule & EmbindModule;
export default function MainModuleFactory (options?: unknown): Promise<MainModule>;
