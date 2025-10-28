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

  const cannyImage = vektorModule.detectEdges(sourceImage, -1);
  const cannyImageData = convertColorImageToImageData(cannyImage);

  const curvesVector = vektorModule.traceEdges(cannyImage);
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
    { imageData: sourceImageData, stageName: "Source" },
    { imageData: cannyImageData, stageName: "Canny Result" },
    { imageData: plotImageData, stageName: "Plot Result" },
  ];

  // nasty
  sourceImage.delete();
  cannyImage.delete();
  curvesVector.delete();
  plotImage.delete();

  return { stages, curves };
}
