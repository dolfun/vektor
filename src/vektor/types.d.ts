// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
interface WasmModule {
}

type EmbindString = ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string;
export interface ClassHandle {
  isAliasOf(other: ClassHandle): boolean;
  delete(): void;
  deleteLater(): this;
  isDeleted(): boolean;
  // @ts-ignore - If targeting lower than ESNext, this symbol might not exist.
  [Symbol.dispose](): void;
  clone(): this;
}
export interface BackgroundColorValue<T extends number> {
  value: T;
}
export type BackgroundColor = BackgroundColorValue<0>|BackgroundColorValue<1>;

export interface DesmosColorValue<T extends number> {
  value: T;
}
export type DesmosColor = DesmosColorValue<0>|DesmosColorValue<1>;

export interface Pipeline extends ClassHandle {
  readonly config: PipelineConfig;
  readonly imageViews: any;
  readonly curves: any;
  setConfig(_0: PipelineConfig): void;
  setSourceImage(_0: any): void;
}

export type PipelineConfig = {
  kernelSize: number,
  nrIterations: number,
  takePercentile: number,
  plotScale: number,
  backgroundColor: BackgroundColor,
  desmosColor: DesmosColor
};

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
  p3: Vec2f,
  color: Vec3f
};

export type ImageView = {
  name: EmbindString,
  width: number,
  height: number,
  data: any
};

interface EmbindModule {
  BackgroundColor: {black: BackgroundColorValue<0>, white: BackgroundColorValue<1>};
  DesmosColor: {solid: DesmosColorValue<0>, colorful: DesmosColorValue<1>};
  Pipeline: {
    new(): Pipeline;
  };
  defaultPipelineConfig: PipelineConfig;
}

export type MainModule = WasmModule & EmbindModule;
export default function MainModuleFactory (options?: unknown): Promise<MainModule>;
