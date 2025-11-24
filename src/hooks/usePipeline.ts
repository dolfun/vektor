import {
  useState,
  useEffect,
  useRef,
  useCallback,
  type SetStateAction,
} from "react";
import { useVektorModule } from "./useVektorModule";
import type {
  Pipeline,
  ImageView,
  BezierCurve,
  PipelineConfig,
} from "@/vektor";
import type { ImageData } from "@/utility";

export function usePipeline(): {
  pipelineConfig: PipelineConfig;
  getImageViews: () => ImageView[];
  getCurves: () => BezierCurve[];
  setSourceImage: (image: ImageData) => void;
  setPipelineConfig: (update: SetStateAction<PipelineConfig>) => void;
} {
  const vektorModule = useVektorModule();

  const pipelineRef = useRef<Pipeline | null>(null);
  if (pipelineRef.current === null) {
    pipelineRef.current = new vektorModule.Pipeline();
  }

  const [, forceRerender] = useState(0);

  const [pipelineConfig, setPipelineConfigState] = useState<PipelineConfig>(
    () => pipelineRef.current!.config
  );

  useEffect(() => {
    if (!pipelineRef.current) {
      pipelineRef.current = new vektorModule.Pipeline();
      pipelineRef.current.setConfig(pipelineConfig);
    }

    return () => {
      if (pipelineRef.current) {
        pipelineRef.current.delete();
        pipelineRef.current = null;
      }
    };
  }, []);

  const setPipelineConfig = useCallback(
    (update: SetStateAction<PipelineConfig>) => {
      setPipelineConfigState((prevConfig) => {
        const newConfig =
          update instanceof Function ? update(prevConfig) : update;

        if (pipelineRef.current) {
          pipelineRef.current.setConfig(newConfig);
        }

        return newConfig;
      });
    },
    []
  );

  const setSourceImage = useCallback((image: ImageData) => {
    if (pipelineRef.current) {
      pipelineRef.current.setSourceImage(image);
      forceRerender((v) => v + 1);
    }
  }, []);

  const getImageViews = () => pipelineRef.current!.imageViews;
  const getCurves = () => pipelineRef.current!.curves;

  return {
    pipelineConfig,
    getImageViews,
    getCurves,
    setSourceImage,
    setPipelineConfig,
  };
}
