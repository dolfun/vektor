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
export type Vec3f = {
  r: number,
  g: number,
  b: number
};

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

interface EmbindModule {
  ColorImage: {
    new(_0: number, _1: number): ColorImage;
  };
  GreyscaleImage: {
    new(_0: number, _1: number): GreyscaleImage;
  };
  detectEdges(_0: ColorImage, _1: number): GreyscaleImage;
}

export type MainModule = WasmModule & EmbindModule;
export default function MainModuleFactory (options?: unknown): Promise<MainModule>;
