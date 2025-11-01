import type {
  VektorModule,
  ColorImage,
  GreyscaleImage,
  BinaryImage,
  GradientImage,
} from "@/vektor";

export type ImageData = {
  readonly data: ImageDataArray;
  readonly width: number;
  readonly height: number;
};

export async function getImagePixelsRGBA8(
  src: File | Blob | string
): Promise<ImageData> {
  const url = typeof src === "string" ? src : URL.createObjectURL(src);
  try {
    const image = await new Promise<HTMLImageElement>((res, rej) => {
      const element = new Image();
      element.onload = () => res(element);
      element.onerror = rej as any;
      element.src = url;
    });
    const width = image.naturalWidth,
      height = image.naturalHeight;

    const canvas = document.createElement("canvas");
    canvas.width = width;
    canvas.height = height;

    const ctx = canvas.getContext("2d");
    if (!ctx) throw new Error("2D context unavailable");
    ctx.drawImage(image, 0, 0);
    const { data } = ctx.getImageData(0, 0, width, height);
    return { data, width, height } as const;
  } finally {
    if (typeof src !== "string") URL.revokeObjectURL(url);
  }
}

export function convertImageDataToColorImage(
  vektorModule: VektorModule,
  imageData: ImageData
): ColorImage {
  const { width, height, data } = imageData;
  const colorImage = new vektorModule.ColorImage(width, height);

  let i = 0;
  for (let y = 0; y < height; ++y) {
    for (let x = 0; x < width; ++x, i += 4) {
      const r = data[i] / 255;
      const g = data[i + 1] / 255;
      const b = data[i + 2] / 255;

      colorImage.setPixel(x, y, { r, g, b });
    }
  }

  return colorImage;
}

export function convertColorImageToImageData(
  image: ColorImage | GradientImage | GreyscaleImage | BinaryImage
): ImageData {
  const { width, height } = image;
  const data = new Uint8ClampedArray(width * height * 4);

  let i = 0;
  for (let y = 0; y < height; ++y) {
    for (let x = 0; x < width; ++x, i += 4) {
      let r, g, b;
      const pixel = image.getPixel(x, y);
      if (typeof pixel == "number") {
        r = g = b = pixel;
      } else if ("mag" in pixel) {
        r = g = b = pixel.mag;
      } else {
        r = pixel.r;
        g = pixel.g;
        b = pixel.b;
      }

      data[i] = r * 255;
      data[i + 1] = g * 255;
      data[i + 2] = b * 255;
      data[i + 3] = 255;
    }
  }

  return { data, width, height };
}
