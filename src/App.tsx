import React, { useState, useRef } from "react";
import { Box, Button, Stack } from "@mui/material";

import { ConfigPanel, MultiStageCanvas, FinalStageCanvas } from "@/components";
import { getImagePixelsRGBA8, createStages } from "@/utility";
import type { BezierCurve, VektorModule } from "@/vektor";
import type { Stage } from "@/utility";

export default function App({ vektorModule }: { vektorModule: VektorModule }) {
  const [resultState, setResultState] = useState<{
    stages: Stage[];
    curves: BezierCurve[];
  } | null>(null);
  const [showFinal, setShowFinal] = useState(false);
  const inputRef = useRef<HTMLInputElement | null>(null);

  const onPick: React.ChangeEventHandler<HTMLInputElement> = async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;
    if (!file.type.startsWith("image/")) {
      alert("Please select an image file.");
      e.currentTarget.value = "";
      return;
    }

    const sourceImageData = await getImagePixelsRGBA8(file);
    const { stages, curves } = createStages(vektorModule, sourceImageData);
    setResultState({ stages, curves });
  };

  return (
    <Box sx={{ height: "100vh", width: "100vw", overflow: "hidden" }}>
      {!resultState ? (
        <Box sx={{ height: "100%", display: "grid", placeItems: "center" }}>
          <Button
            variant="contained"
            size="large"
            onClick={() => inputRef.current?.click()}
          >
            Choose Image
          </Button>
          <input
            ref={inputRef}
            type="file"
            accept="image/*"
            hidden
            onChange={onPick}
          />
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
            <ConfigPanel showFinal={showFinal} onToggleFinal={setShowFinal} />
          </Box>
        </Stack>
      )}
    </Box>
  );
}
