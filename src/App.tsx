import React, { useState, useRef, useEffect } from "react";
import { Box, Button, Stack } from "@mui/material";

import { ConfigPanel, MultiStageCanvas, FinalStageCanvas } from "@/components";
import { getImagePixelsRGBA8, createStages } from "@/utility";
import type { BezierCurve, VektorModule } from "@/vektor";
import type { StagesParams, Stage, ImageData } from "@/utility";
import { defaultStageParams } from "@/utility";

export default function App({ vektorModule }: { vektorModule: VektorModule }) {
  const [stageParams, setStageParams] =
    useState<StagesParams>(defaultStageParams);
  const [sourceImageData, setSourceImageData] = useState<ImageData | null>(
    null
  );
  const [resultState, setResultState] = useState<{
    stages: Stage[];
    curves: BezierCurve[];
  } | null>(null);
  const [showFinal, setShowFinal] = useState(false);
  const inputRef = useRef<HTMLInputElement | null>(null);
  const crashNotifiedRef = useRef(false);

  const onPick: React.ChangeEventHandler<HTMLInputElement> = async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;
    if (!file.type.startsWith("image/")) {
      alert("Please select an image file.");
      e.currentTarget.value = "";
      return;
    }
    const imgData = await getImagePixelsRGBA8(file);
    setSourceImageData(imgData);
  };

  useEffect(() => {
    if (!sourceImageData) return;
    try {
      const { stages, curves } = createStages(
        vektorModule,
        sourceImageData,
        stageParams
      );
      setResultState({ stages, curves });
      crashNotifiedRef.current = false;
    } catch (err) {
      console.error("createStages failed:", err);
      if (!crashNotifiedRef.current) {
        alert("Oops — the page crashed!");
        crashNotifiedRef.current = true;
        window.location.reload();
      }
    }
  }, [stageParams, sourceImageData, vektorModule]);

  return (
    <Box sx={{ height: "100vh", width: "100vw", overflow: "hidden" }}>
      <input
        ref={inputRef}
        type="file"
        accept="image/*"
        hidden
        onChange={onPick}
      />
      {!resultState ? (
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
              <FinalStageCanvas curves={resultState.curves} />
            ) : (
              <MultiStageCanvas stages={resultState.stages} />
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
