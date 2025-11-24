import React, { useState, useRef } from "react";
import { Box, Button, Stack } from "@mui/material";
import { ConfigPanel, MultiStageCanvas, FinalStageCanvas } from "@/components";
import { getImagePixelsRGBA8 } from "@/utility";
import { useVektorModule, usePipeline } from "./hooks";

export default function App() {
  const vektorModule = useVektorModule();
  const [ready, setReady] = useState(false);
  const [showFinal, setShowFinal] = useState(false);
  const inputRef = useRef<HTMLInputElement | null>(null);

  const {
    pipelineConfig,
    getImageViews,
    getCurves,
    setSourceImage,
    setPipelineConfig,
  } = usePipeline();

  const onPick: React.ChangeEventHandler<HTMLInputElement> = async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;
    if (!file.type.startsWith("image/")) {
      alert("Please select an image file.");
      e.currentTarget.value = "";
      return;
    }

    const imageData = await getImagePixelsRGBA8(file);
    setSourceImage(imageData);
    setReady(true);
  };

  return (
    <Box sx={{ height: "100vh", width: "100vw", overflow: "hidden" }}>
      <input
        ref={inputRef}
        type="file"
        accept="image/*"
        hidden
        onChange={onPick}
      />

      {!ready ? (
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
                getCurves={getCurves}
                desmosColor={
                  pipelineConfig.desmosColor === vektorModule.DesmosColor.solid
                    ? "solid"
                    : "colorful"
                }
                invertedColors={
                  pipelineConfig.backgroundColor ===
                  vektorModule.BackgroundColor.black
                }
              />
            ) : (
              <MultiStageCanvas getImageViews={getImageViews} />
            )}
          </Box>
          <Box sx={{ height: "100%" }}>
            <ConfigPanel
              showFinal={showFinal}
              onToggleFinal={setShowFinal}
              pipelineConfig={pipelineConfig}
              setPipelineConfig={setPipelineConfig}
              inputRef={inputRef}
            />
          </Box>
        </Stack>
      )}
    </Box>
  );
}
