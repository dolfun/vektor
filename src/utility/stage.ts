import {
  convertImageDataToColorImage,
  convertColorImageToImageData,
} from "./image";
import type { ImageData } from "./image";
import type { VektorModule } from "@/vektor";

export type Stage = {
  imageData: ImageData;
  stageName: string;
};

export function createStages(
  vektorModule: VektorModule,
  sourceImageData: ImageData
): Stage[] {
  const sourceImage = convertImageDataToColorImage(
    vektorModule,
    sourceImageData
  );

  const resultImage = vektorModule.detectEdges(sourceImage, -1);
  const resultImageData = convertColorImageToImageData(resultImage);

  const stages: Stage[] = [
    { imageData: sourceImageData, stageName: "Source" },
    { imageData: resultImageData, stageName: "Result" },
  ];

  sourceImage.delete();
  resultImage.delete();

  return stages;
}
