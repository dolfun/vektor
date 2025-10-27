import React, { useState, useRef } from "react";
import { Box, Button, Stack } from "@mui/material";

import { ConfigPanel, MultiStageCanvas, FinalStageCanvas } from "@/components";
import { getImagePixelsRGBA8, createStages } from "@/utility";
import type { VektorModule } from "@/vektor";
import type { Stage } from "@/utility";

export default function App({ vektorModule }: { vektorModule: VektorModule }) {
  const [stages, setStages] = useState<Stage[] | null>(null);
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

    const imageData = await getImagePixelsRGBA8(file);
    const stages = createStages(vektorModule, imageData);
    setStages(stages);
  };

  return (
    <Box sx={{ height: "100vh", width: "100vw", overflow: "hidden" }}>
      {!stages ? (
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
              <FinalStageCanvas />
            ) : (
              <MultiStageCanvas stages={stages} />
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
