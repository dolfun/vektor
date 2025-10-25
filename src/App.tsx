import React, { useState, useRef } from "react";

import { ThemeProvider, createTheme } from "@mui/material/styles";
import CssBaseline from "@mui/material/CssBaseline";
import { Box, Button, Stack } from "@mui/material";

import {
  ConfigPanel,
  MultiStageCanvas,
  FinalStageCanvas,
  type Stage,
} from "@/components";
import { getImagePixelsRGBA8 } from "@/utility";

const darkTheme = createTheme({
  palette: { mode: "dark" },
});

function DarkTheme({ children }: { children: React.ReactNode }) {
  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      {children}
    </ThemeProvider>
  );
}

export default function App() {
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
    const img = await getImagePixelsRGBA8(file);
    const triplet = [1, 2, 3].map((i) => ({ ...img, stageName: `Stage ${i}` }));
    setStages(triplet);
    setShowFinal(false);
    e.currentTarget.value = "";
  };

  return (
    <DarkTheme>
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
    </DarkTheme>
  );
}
