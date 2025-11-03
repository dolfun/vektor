import {
  Accordion,
  AccordionDetails,
  AccordionSummary,
  Box,
  Button,
  Checkbox,
  Divider,
  FormControlLabel,
  Paper,
  Slider,
  Stack,
  Typography,
} from "@mui/material";
import React from "react";
import type { StagesParams } from "@/utility";
import { defaultStageParams } from "@/utility";

type Props = {
  showFinal: boolean;
  onToggleFinal: (v: boolean) => void;
  stageParams: StagesParams;
  setStageParams: React.Dispatch<React.SetStateAction<StagesParams>>;
  inputRef: React.RefObject<HTMLInputElement | null>;
};

export function ConfigPanel({
  showFinal,
  onToggleFinal,
  stageParams,
  setStageParams,
  inputRef,
}: Props) {
  const handleParamChange =
    <K extends keyof StagesParams>(key: K) =>
    (_: Event, value: number | number[]) => {
      const v = Array.isArray(value) ? value[0] : value;
      setStageParams((prev) => ({ ...prev, [key]: v as StagesParams[K] }));
    };

  const marksPlotScale = [
    { value: 1, label: "1x" },
    { value: 2, label: "2x" },
    { value: 3, label: "3x" },
    { value: 4, label: "4x" },
  ];

  const handleReset = () => {
    setStageParams(defaultStageParams);
  };

  return (
    <Paper
      elevation={2}
      sx={{
        width: { xs: "100%", md: "20vw" },
        minWidth: 260,
        p: 2,
        height: "100%",
        overflow: "auto",
      }}
    >
      <Stack spacing={1}>
        <Typography variant="subtitle1" sx={{ textAlign: "center" }}>
          Configuration
        </Typography>
        <Divider />

        <Accordion disableGutters>
          <AccordionSummary expandIcon={<span>▾</span>}>
            <Typography variant="body1">Blur Stage</Typography>
          </AccordionSummary>
          <AccordionDetails>
            <Stack spacing={2} sx={{ px: 1 }}>
              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  # Iterations
                </Typography>
                <Slider
                  value={stageParams.nrIterations}
                  onChange={(_, v) =>
                    handleParamChange("nrIterations")(
                      _,
                      Math.round(v as number)
                    )
                  }
                  min={1}
                  max={5}
                  step={1}
                  valueLabelDisplay="auto"
                  aria-label="Number of Iterations"
                />
              </Box>
            </Stack>
          </AccordionDetails>
        </Accordion>

        <Accordion disableGutters>
          <AccordionSummary expandIcon={<span>▾</span>}>
            <Typography variant="body1">Hysteresis Stage</Typography>
          </AccordionSummary>
          <AccordionDetails>
            <Stack spacing={2} sx={{ px: 1 }}>
              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  Take Percentile
                </Typography>
                <Slider
                  value={stageParams.takePercentile}
                  onChange={handleParamChange("takePercentile")}
                  min={0}
                  max={1}
                  step={0.01}
                  valueLabelDisplay="auto"
                  aria-label="Take Percentile"
                />
              </Box>
            </Stack>
          </AccordionDetails>
        </Accordion>

        <Accordion disableGutters>
          <AccordionSummary expandIcon={<span>▾</span>}>
            <Typography variant="body1">Final Result</Typography>
          </AccordionSummary>
          <AccordionDetails>
            <Stack spacing={2} sx={{ px: 1 }}>
              <Box
                sx={{
                  display: "flex",
                  justifyContent: "center",
                  alignItems: "center",
                }}
              >
                <FormControlLabel
                  control={
                    <Checkbox
                      checked={showFinal}
                      onChange={(e) => onToggleFinal(e.target.checked)}
                    />
                  }
                  label={
                    <Typography variant="subtitle2">
                      Show Desmos Plot
                    </Typography>
                  }
                />
              </Box>

              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  Plot Scale
                </Typography>
                <Slider
                  value={stageParams.plotScale}
                  onChange={(_, v) =>
                    handleParamChange("plotScale")(_, Math.round(v as number))
                  }
                  min={1}
                  max={4}
                  step={1}
                  marks={marksPlotScale}
                  valueLabelDisplay="auto"
                  aria-label="Plot Scale"
                />
              </Box>
            </Stack>
          </AccordionDetails>
        </Accordion>

        <Box sx={{ pt: 2 }}>
          <Stack spacing={1}>
            <Button
              variant="outlined"
              fullWidth
              onClick={handleReset}
              aria-label="Reset parameters to default"
            >
              Reset to Defaults
            </Button>
            <Button
              variant="contained"
              fullWidth
              onClick={() => inputRef.current?.click()}
              aria-label="Upload Image"
            >
              UPLOAD IMAGE
            </Button>
          </Stack>
        </Box>
      </Stack>
    </Paper>
  );
}
