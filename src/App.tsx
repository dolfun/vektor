import React, { useEffect, useMemo, useRef, useState } from "react";
import { Box, Button, Stack } from "@mui/material";
import { ConfigPanel, MultiStageCanvas, FinalStageCanvas } from "@/components";
import { getImagePixelsRGBA8 } from "@/utility";
import type { VektorModule } from "@/vektor";
import type { ImageData } from "@/utility";
import {
  StagesParams,
  defaultStageParams,
  IDX_SOURCE,
  IDX_CURVES,
  PARAM_TO_INDEX,
  StageTuple,
  ColoredBezier,
  initEmptyStageTuples,
  buildPipelineFrom,
  safeDelete,
  STAGE_NAMES,
  IMAGE_STAGE_INDEXES,
} from "@/utility";

export default function App({ vektorModule }: { vektorModule: VektorModule }) {
  const [stageParams, setStageParams] =
    useState<StagesParams>(defaultStageParams);
  const [sourceImageData, setSourceImageData] = useState<ImageData | null>(
    null
  );
  const [stageTuples, setStageTuples] = useState<StageTuple[]>(
    initEmptyStageTuples()
  );
  const [curves, setCurves] = useState<ColoredBezier[]>([]);
  const [showFinal, setShowFinal] = useState(false);

  const inputRef = useRef<HTMLInputElement | null>(null);
  const prevParamsRef = useRef<StagesParams>(stageParams);
  const lastGoodStagesRef = useRef<
    { imageData: ImageData; stageName: string }[] | null
  >(null);
  const tuplesRef = useRef(stageTuples);

  useEffect(() => {
    tuplesRef.current = stageTuples;
  }, [stageTuples]);

  useEffect(() => {
    return () => {
      for (const t of tuplesRef.current) safeDelete(t.obj);
    };
  }, []);

  const recomputeFrom = (startIndex: number) => {
    if (!sourceImageData) return;

    const { nextTuples, curves: newCurves } = buildPipelineFrom(
      vektorModule,
      sourceImageData,
      stageParams,
      stageTuples,
      startIndex
    );

    setStageTuples((prev) => {
      for (let i = 0; i < prev.length; i++) {
        if (prev[i].obj && prev[i].obj !== nextTuples[i].obj) {
          safeDelete(prev[i].obj);
        }
      }
      return nextTuples;
    });

    setCurves(startIndex > IDX_CURVES && curves.length ? curves : newCurves);
  };

  const onPick: React.ChangeEventHandler<HTMLInputElement> = async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;
    if (!file.type.startsWith("image/")) {
      alert("Please select an image file.");
      e.currentTarget.value = "";
      return;
    }
    const imgData = await getImagePixelsRGBA8(file);

    setStageTuples((prev) => {
      const cleared = prev.map((x) => ({ ...x }));
      for (const t of cleared) {
        safeDelete(t.obj);
        t.obj = null;
        t.data = null;
      }
      return cleared;
    });

    setCurves([]);
    lastGoodStagesRef.current = null;
    setSourceImageData(imgData);
  };

  useEffect(() => {
    if (sourceImageData) recomputeFrom(IDX_SOURCE);
  }, [sourceImageData]);

  useEffect(() => {
    const prev = prevParamsRef.current;
    let earliest = Infinity;

    for (const k in stageParams) {
      const key = k as keyof StagesParams;
      if (stageParams[key] !== prev[key]) {
        const idx = PARAM_TO_INDEX[key];
        if (idx < earliest) earliest = idx;
      }
    }

    prevParamsRef.current = stageParams;

    if (!isFinite(earliest) || !sourceImageData) return;
    recomputeFrom(earliest);
  }, [stageParams]);

  const stagesForView = useMemo(() => {
    const imgs = IMAGE_STAGE_INDEXES.map((i) => ({ i, t: stageTuples[i] }))
      .filter(({ t }) => !!t.data)
      .map(({ i, t }) => ({
        imageData: t.data as ImageData,
        stageName: STAGE_NAMES[i],
      }));

    const allReady = imgs.length === IMAGE_STAGE_INDEXES.length;
    if (allReady) {
      lastGoodStagesRef.current = imgs;
      return imgs;
    }
    return lastGoodStagesRef.current ?? [];
  }, [stageTuples]);

  const hasStages = stagesForView.length === IMAGE_STAGE_INDEXES.length;

  return (
    <Box sx={{ height: "100vh", width: "100vw", overflow: "hidden" }}>
      <input
        ref={inputRef}
        type="file"
        accept="image/*"
        hidden
        onChange={onPick}
      />

      {!hasStages ? (
        <Box sx={{ height: "100%", display: "grid", placeItems: "center" }}>
          <Button
            variant="contained"
            size="large"
            onClick={() => inputRef.current?.click()}
          >
            UPLOAD IMAGE
          </Button>
        </Box>
      ) : (
        <Stack
          direction={{ xs: "column", md: "row" }}
          sx={{ height: "100%", p: 2, gap: 2 }}
        >
          <Box
            sx={{
              flex: 1,
              minWidth: 0,
              height: "100%",
              display: "flex",
              flexDirection: "column",
            }}
          >
            {showFinal ? (
              <FinalStageCanvas
                curves={curves}
                desmosColor={stageParams.desmosColor}
                invertedColors={stageParams.backgroundColor === "black"}
              />
            ) : (
              <MultiStageCanvas stages={stagesForView} />
            )}
          </Box>
          <Box sx={{ height: "100%" }}>
            <ConfigPanel
              showFinal={showFinal}
              onToggleFinal={setShowFinal}
              stageParams={stageParams}
              setStageParams={setStageParams}
              inputRef={inputRef}
            />
          </Box>
        </Stack>
      )}
    </Box>
  );
}
