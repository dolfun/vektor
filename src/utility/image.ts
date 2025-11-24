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
