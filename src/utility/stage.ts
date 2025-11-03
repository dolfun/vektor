import {
  convertImageDataToColorImage,
  convertColorImageToImageData,
} from "./image";
import type { ImageData } from "./image";
import type { VektorModule, BezierCurve } from "@/vektor";

export type StagesParams = {
  nrIterations: number;
  takePercentile: number;
  plotScale: number;
};

export const defaultStageParams: StagesParams = {
  nrIterations: 1,
  takePercentile: 0.25,
  plotScale: 2,
};

export type Stage = {
  imageData: ImageData;
  stageName: string;
};

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
      convertImageDataToColorImage(vektorModule, sourceImageData)
    );

    const blurFactor = 1.0;
    const blurredImage = track(
      vektorModule.applyAdaptiveBlur(
        sourceImage,
        blurFactor,
        stageParams.nrIterations
      )
    );

    const gradientImage = track(vektorModule.computeGradient(blurredImage));

    const thinnedImage = track(vektorModule.thinEdges(gradientImage));

    const threshold = vektorModule.computeThreshold(thinnedImage, 256);
    const cannyResultImage = track(
      vektorModule.applyHysteresis(
        thinnedImage,
        threshold.first,
        threshold.second,
        stageParams.takePercentile
      )
    );

    const curvesVector = track(vektorModule.traceEdges(cannyResultImage));

    const plotImage = track(
      vektorModule.renderCurves(
        sourceImageData.width * stageParams.plotScale,
        sourceImageData.height * stageParams.plotScale,
        curvesVector
      )
    );

    const blurredImageData = convertColorImageToImageData(blurredImage);
    const gradientImageData = convertColorImageToImageData(gradientImage);
    const thinnedImageData = convertColorImageToImageData(thinnedImage);
    const cannyImageData = convertColorImageToImageData(cannyResultImage);
    const plotImageData = convertColorImageToImageData(plotImage);

    const curves: BezierCurve[] = Array.from(
      { length: curvesVector.size() },
      (_, i) => curvesVector.get(i)!
    );

    const stages: Stage[] = [
      { imageData: sourceImageData, stageName: "Source Image" },
      { imageData: blurredImageData, stageName: "Blurred Image" },
      { imageData: gradientImageData, stageName: "Gradient Image" },
      { imageData: thinnedImageData, stageName: "Thinned Image" },
      { imageData: cannyImageData, stageName: "Canny Result" },
      { imageData: plotImageData, stageName: "Plot Result" },
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
