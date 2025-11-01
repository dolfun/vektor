import {
  convertImageDataToColorImage,
  convertColorImageToImageData,
} from "./image";
import type { ImageData } from "./image";
import type { VektorModule, BezierCurve } from "@/vektor";

export type Stage = {
  imageData: ImageData;
  stageName: string;
};

export function createStages(
  vektorModule: VektorModule,
  sourceImageData: ImageData
): { stages: Stage[]; curves: BezierCurve[] } {
  const sourceImage = convertImageDataToColorImage(
    vektorModule,
    sourceImageData
  );

  const blurredImage = vektorModule.applyAdaptiveBlur(sourceImage, 1.0, 1);
  const blurredImageData = convertColorImageToImageData(blurredImage);

  const gradientImage = vektorModule.computeGradient(blurredImage);
  const gradientImageData = convertColorImageToImageData(gradientImage);

  const thinnedImage = vektorModule.thinEdges(gradientImage);
  const thinnedImageData = convertColorImageToImageData(thinnedImage);

  const high_threshold = vektorModule.computeThreshold(thinnedImage);
  const low_threshold = high_threshold / 2;
  const cannyResultImage = vektorModule.applyHysteresis(
    thinnedImage,
    high_threshold,
    low_threshold
  );
  const cannyImageData = convertColorImageToImageData(cannyResultImage);

  const curvesVector = vektorModule.traceEdges(cannyResultImage);
  const curves: BezierCurve[] = Array.from(
    { length: curvesVector.size() },
    (_, i) => curvesVector.get(i)!
  );

  const plotImage = vektorModule.renderCurves(
    sourceImageData.width,
    sourceImageData.height,
    curvesVector
  );
  const plotImageData = convertColorImageToImageData(plotImage);

  const stages: Stage[] = [
    { imageData: sourceImageData, stageName: "Source Image" },
    { imageData: blurredImageData, stageName: "Blurred Image" },
    { imageData: gradientImageData, stageName: "Gradient Image" },
    { imageData: thinnedImageData, stageName: "Thinned Image" },
    { imageData: cannyImageData, stageName: "Canny Result" },
    { imageData: plotImageData, stageName: "Plot Result" },
  ];

  // nasty
  sourceImage.delete();
  blurredImage.delete();
  gradientImage.delete();
  thinnedImage.delete();
  cannyResultImage.delete();
  curvesVector.delete();
  plotImage.delete();

  return { stages, curves };
}
