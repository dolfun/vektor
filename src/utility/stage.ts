import {
  convertImageDataToColorImage,
  convertColorImageToImageData,
} from "@/utility";
import type { ImageData } from "@/utility";
import type {
  VektorModule,
  ColorImage,
  GradientImage,
  GreyscaleImage,
  BinaryImage,
  BezierCurveArray,
  BezierCurve,
  Vec3f,
} from "@/vektor";

export type StagesParams = {
  kernelSize: number;
  nrIterations: number;
  takePercentile: number;
  plotScale: number;
  backgroundColor: "black" | "white";
  desmosColor: "solid" | "auto";
};

export const defaultStageParams: StagesParams = {
  kernelSize: 1,
  nrIterations: 1,
  takePercentile: 0.25,
  plotScale: 1,
  backgroundColor: "black",
  desmosColor: "auto",
};

export const IDX_SOURCE = 0;
export const IDX_BLUR = 1;
export const IDX_GRADIENT = 2;
export const IDX_THINNED = 3;
export const IDX_CANNY = 4;
export const IDX_CURVES = 5;
export const IDX_GREYSCALE_PLOT = 6;
export const IDX_COLOR_PLOT = 7;

export const STAGE_NAMES = [
  "Source Image",
  "Blurred Image",
  "Gradient Image",
  "Thinned Image",
  "Canny Result",
  "Curves",
  "Greyscale Plot",
  "Color Plot",
] as const;

export const IMAGE_STAGE_INDEXES = [
  IDX_SOURCE,
  IDX_BLUR,
  IDX_GRADIENT,
  IDX_THINNED,
  IDX_CANNY,
  IDX_GREYSCALE_PLOT,
  IDX_COLOR_PLOT,
] as const;

export type StageObj =
  | ColorImage
  | GradientImage
  | GreyscaleImage
  | BinaryImage
  | BezierCurveArray;

export type StageTuple = {
  obj: StageObj | null;
  data: ImageData | null;
  name: string;
};

export type ColoredBezier = BezierCurve & { color: Vec3f };
export type ViewStage = { imageData: ImageData; stageName: string };

export const PARAM_TO_INDEX: Record<keyof StagesParams, number> = {
  kernelSize: IDX_BLUR,
  nrIterations: IDX_BLUR,
  takePercentile: IDX_CANNY,
  plotScale: IDX_GREYSCALE_PLOT,
  backgroundColor: IDX_GREYSCALE_PLOT,
  desmosColor: Number.POSITIVE_INFINITY,
};

function timeIt<T>(label: string, fn: () => T): T {
  const t0 = performance.now();
  try {
    return fn();
  } finally {
    const t1 = performance.now();
    console.log(`[vektor] ${label}: ${(t1 - t0).toFixed(2)} ms`);
  }
}

export function initEmptyStageTuples(): StageTuple[] {
  return STAGE_NAMES.map((name) => ({ obj: null, data: null, name }));
}

export function safeDelete(
  obj: { delete(): void; isDeleted?: () => boolean } | null | undefined
) {
  if (!obj) return;
  try {
    if (typeof obj.isDeleted === "function" && obj.isDeleted()) return;
  } catch {}
  try {
    obj.delete();
  } catch {}
}

export function disposeFrom(startIndex: number, tuples: StageTuple[]) {
  for (let i = startIndex; i < tuples.length; i++) {
    safeDelete(tuples[i].obj);
    tuples[i] = { ...tuples[i], obj: null, data: null };
  }
}

export function clampScaledSize(
  w: number,
  h: number,
  s: number
): { W: number; H: number } {
  return {
    W: Math.max(1, Math.round(w * s)),
    H: Math.max(1, Math.round(h * s)),
  };
}

/** tiny setter to avoid repeating safeDelete + struct copy */
function setStage(
  arr: StageTuple[],
  idx: number,
  obj: StageObj | null,
  data: ImageData | null
) {
  safeDelete(arr[idx].obj);
  arr[idx] = { ...arr[idx], obj, data };
}

export function buildPipelineFrom(
  vektorModule: VektorModule,
  sourceImageData: ImageData,
  stageParams: StagesParams,
  prevTuples: StageTuple[],
  startIndex: number
): { nextTuples: StageTuple[]; curves: ColoredBezier[] } {
  const next = prevTuples.map((x) => ({ ...x }));
  const greyscaleBackground =
    stageParams.backgroundColor === "black" ? 0.0 : 1.0;
  const colorBackground: Vec3f =
    stageParams.backgroundColor === "black"
      ? { r: 0, g: 0, b: 0 }
      : { r: 1, g: 1, b: 1 };

  const toCleanup: StageObj[] = [];
  const track = <T extends StageObj>(h: T) => (toCleanup.push(h), h);

  try {
    disposeFrom(startIndex, next);

    if (startIndex <= IDX_SOURCE || !next[IDX_SOURCE].obj) {
      const srcObj: ColorImage = convertImageDataToColorImage(
        vektorModule,
        sourceImageData
      );
      setStage(next, IDX_SOURCE, srcObj, sourceImageData);
    }

    if (startIndex <= IDX_BLUR || !next[IDX_BLUR].obj) {
      const srcObj = next[IDX_SOURCE].obj as ColorImage;
      const blurred = track(
        timeIt("applyAdaptiveBlur", () =>
          vektorModule.applyAdaptiveBlur(
            srcObj,
            1.0,
            stageParams.kernelSize,
            stageParams.nrIterations
          )
        )
      );
      setStage(next, IDX_BLUR, blurred, convertColorImageToImageData(blurred));
    }

    if (startIndex <= IDX_GRADIENT || !next[IDX_GRADIENT].obj) {
      const blurred = next[IDX_BLUR].obj as ColorImage;
      const gradient = track(
        timeIt("computeGradient", () => vektorModule.computeGradient(blurred))
      );
      setStage(
        next,
        IDX_GRADIENT,
        gradient,
        convertColorImageToImageData(gradient)
      );
    }

    if (startIndex <= IDX_THINNED || !next[IDX_THINNED].obj) {
      const gradient = next[IDX_GRADIENT].obj as GradientImage;
      const thinned = track(
        timeIt("thinEdges", () => vektorModule.thinEdges(gradient))
      );
      setStage(
        next,
        IDX_THINNED,
        thinned,
        convertColorImageToImageData(thinned)
      );
    }

    if (startIndex <= IDX_CANNY || !next[IDX_CANNY].obj) {
      const thinned = next[IDX_THINNED].obj as GreyscaleImage;
      const thr = timeIt("computeThreshold", () =>
        vektorModule.computeThreshold(thinned, 256)
      );
      const canny = track(
        timeIt("applyHysteresis", () =>
          vektorModule.applyHysteresis(
            thinned,
            thr.first,
            thr.second,
            stageParams.takePercentile
          )
        )
      );
      setStage(next, IDX_CANNY, canny, convertColorImageToImageData(canny));
    }

    if (startIndex <= IDX_CURVES || !next[IDX_CURVES].obj) {
      const canny = next[IDX_CANNY].obj as BinaryImage;
      const curvesArray = track(
        timeIt("traceEdges", () => vektorModule.traceEdges(canny))
      );
      setStage(next, IDX_CURVES, curvesArray, null);
    }

    const curvesArray = next[IDX_CURVES].obj as BezierCurveArray;
    const sourceObj = next[IDX_SOURCE].obj as ColorImage;

    const curves: ColoredBezier[] = Array.from(
      { length: curvesArray.size() },
      (_, i) => {
        const c = curvesArray.get(i)!;
        const color = vektorModule.computeCurveColor(c, sourceObj);
        return { ...c, color };
      }
    );

    const { W, H } = clampScaledSize(
      sourceImageData.width,
      sourceImageData.height,
      stageParams.plotScale
    );

    if (startIndex <= IDX_GREYSCALE_PLOT || !next[IDX_GREYSCALE_PLOT].data) {
      const gsPlot = timeIt("renderCurvesGreyscale", () =>
        vektorModule.renderCurvesGreyscale(
          W,
          H,
          curvesArray,
          greyscaleBackground
        )
      );
      const gsData = convertColorImageToImageData(gsPlot);
      safeDelete(gsPlot);
      setStage(next, IDX_GREYSCALE_PLOT, null, gsData);
    }

    if (startIndex <= IDX_COLOR_PLOT || !next[IDX_COLOR_PLOT].data) {
      const colorPlot = timeIt("renderCurvesColor", () =>
        vektorModule.renderCurvesColor(
          W,
          H,
          curvesArray,
          sourceObj,
          colorBackground
        )
      );
      const colorData = convertColorImageToImageData(colorPlot);
      safeDelete(colorPlot);
      setStage(next, IDX_COLOR_PLOT, null, colorData);
    }

    return { nextTuples: next, curves };
  } catch (e) {
    for (const h of toCleanup) safeDelete(h);
    throw e;
  }
}
