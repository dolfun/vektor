import {
  convertImageDataToColorImage,
  convertColorImageToImageData,
} from "./image";
import type { ImageData } from "./image";
import type { VektorModule, BezierCurve, Vec3f } from "@/vektor";

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

export type Stage = {
  imageData: ImageData;
  stageName: string;
};

function timeIt<T>(label: string, fn: () => T): T {
  const t0 = performance.now();
  try {
    return fn();
  } finally {
    const t1 = performance.now();
    console.log(`[Stages] ${label}: ${(t1 - t0).toFixed(2)} ms`);
  }
}

export function createStages(
  vektorModule: VektorModule,
  sourceImageData: ImageData,
  stageParams: StagesParams
): { stages: Stage[]; curves: BezierCurve[] } {
  const disposables: Array<{ delete(): void }> = [];
  const track = <T extends { delete(): void }>(x: T): T => {
    disposables.push(x);
    return x;
  };

  try {
    const sourceImage = track(
      timeIt("convertImageDataToColorImage", () =>
        convertImageDataToColorImage(vektorModule, sourceImageData)
      )
    );

    const blurFactor = 1.0;

    const blurredImage = track(
      timeIt("vektorModule.applyAdaptiveBlur", () =>
        vektorModule.applyAdaptiveBlur(
          sourceImage,
          blurFactor,
          stageParams.kernelSize,
          stageParams.nrIterations
        )
      )
    );

    const gradientImage = track(
      timeIt("vektorModule.computeGradient", () =>
        vektorModule.computeGradient(blurredImage)
      )
    );

    const thinnedImage = track(
      timeIt("vektorModule.thinEdges", () =>
        vektorModule.thinEdges(gradientImage)
      )
    );

    const threshold = timeIt("vektorModule.computeThreshold", () =>
      vektorModule.computeThreshold(thinnedImage, 256)
    );

    const cannyResultImage = track(
      timeIt("vektorModule.applyHysteresis", () =>
        vektorModule.applyHysteresis(
          thinnedImage,
          threshold.first,
          threshold.second,
          stageParams.takePercentile
        )
      )
    );

    const curvesVector = track(
      timeIt("vektorModule.traceEdges", () =>
        vektorModule.traceEdges(cannyResultImage)
      )
    );

    const greyscaleImageBackground =
      stageParams.backgroundColor === "black" ? 0.0 : 1.0;

    const greyscalePlotImage = track(
      timeIt("vektorModule.renderCurvesGreyscale", () =>
        vektorModule.renderCurvesGreyscale(
          sourceImageData.width * stageParams.plotScale,
          sourceImageData.height * stageParams.plotScale,
          curvesVector,
          greyscaleImageBackground
        )
      )
    );

    const colorImageBackground: Vec3f =
      stageParams.backgroundColor === "black"
        ? { r: 0, g: 0, b: 0 }
        : { r: 1, g: 1, b: 1 };

    const colorPlotImage = track(
      timeIt("vektorModule.renderCurvesColor", () =>
        vektorModule.renderCurvesColor(
          sourceImageData.width * stageParams.plotScale,
          sourceImageData.height * stageParams.plotScale,
          curvesVector,
          sourceImage,
          colorImageBackground
        )
      )
    );

    const {
      blurredImageData,
      gradientImageData,
      thinnedImageData,
      cannyImageData,
      greyscalePlotImageData,
      colorPlotImageData,
    } = timeIt("convertColorImageToImageData block", () => {
      const blurredImageData = convertColorImageToImageData(blurredImage);
      const gradientImageData = convertColorImageToImageData(gradientImage);
      const thinnedImageData = convertColorImageToImageData(thinnedImage);
      const cannyImageData = convertColorImageToImageData(cannyResultImage);
      const greyscalePlotImageData =
        convertColorImageToImageData(greyscalePlotImage);
      const colorPlotImageData = convertColorImageToImageData(colorPlotImage);
      return {
        blurredImageData,
        gradientImageData,
        thinnedImageData,
        cannyImageData,
        greyscalePlotImageData,
        colorPlotImageData,
      };
    });

    const curves: BezierCurve[] = timeIt(
      "vektorModule.computeCurveColor (all curves)",
      () =>
        Array.from({ length: curvesVector.size() }, (_, i) => {
          const curve = curvesVector.get(i)!;
          return {
            ...curve,
            color: vektorModule.computeCurveColor(curve, sourceImage),
          };
        })
    );

    const stages: Stage[] = [
      { imageData: sourceImageData, stageName: "Source Image" },
      { imageData: blurredImageData, stageName: "Blurred Image" },
      { imageData: gradientImageData, stageName: "Gradient Image" },
      { imageData: thinnedImageData, stageName: "Thinned Image" },
      { imageData: cannyImageData, stageName: "Canny Result" },
      { imageData: greyscalePlotImageData, stageName: "Greyscale Plot" },
      { imageData: colorPlotImageData, stageName: "Color Plot" },
    ];

    return { stages, curves };
  } finally {
    for (let i = disposables.length - 1; i >= 0; i--) {
      try {
        disposables[i].delete();
      } catch {}
    }
  }
}
