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
  Radio,
  RadioGroup,
} from "@mui/material";
import React from "react";

import { useVektorModule } from "@/hooks";
import type { PipelineConfig } from "@/vektor";

type Props = {
  showFinal: boolean;
  onToggleFinal: (v: boolean) => void;
  pipelineConfig: PipelineConfig;
  setPipelineConfig: React.Dispatch<React.SetStateAction<PipelineConfig>>;
  inputRef: React.RefObject<HTMLInputElement | null>;
};

const marksPlotScale = [
  { value: 1, label: "1x" },
  { value: 2, label: "2x" },
  { value: 3, label: "3x" },
  { value: 4, label: "4x" },
];

export function ConfigPanel({
  showFinal,
  onToggleFinal,
  pipelineConfig,
  setPipelineConfig,
  inputRef,
}: Props) {
  const vektorModule = useVektorModule();

  const handleSliderChange =
    (key: keyof PipelineConfig, shouldRound: boolean = false) =>
    (_: Event, value: number | number[]) => {
      let paramValue = Array.isArray(value) ? value[0] : value;

      if (shouldRound) {
        paramValue = Math.round(paramValue);
      }

      setPipelineConfig((prev) => ({
        ...prev,
        [key]: paramValue,
      }));
    };

  const handleReset = () => {
    setPipelineConfig(vektorModule.defaultPipelineConfig);
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
            <Typography variant="body1">Denoising</Typography>
          </AccordionSummary>
          <AccordionDetails>
            <Stack spacing={2} sx={{ px: 1 }}>
              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  Kernel Size
                </Typography>
                <Slider
                  value={pipelineConfig.kernelSize}
                  onChange={handleSliderChange("kernelSize", true)}
                  min={1}
                  max={3}
                  step={1}
                  valueLabelDisplay="auto"
                  aria-label="Kernel Size"
                />
              </Box>

              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  # Iterations
                </Typography>
                <Slider
                  value={pipelineConfig.nrIterations}
                  onChange={handleSliderChange("nrIterations", true)}
                  min={1}
                  max={4}
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
            <Typography variant="body1">Hysteresis</Typography>
          </AccordionSummary>
          <AccordionDetails>
            <Stack spacing={2} sx={{ px: 1 }}>
              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  Take Percentile
                </Typography>
                <Slider
                  value={pipelineConfig.takePercentile}
                  onChange={handleSliderChange("takePercentile", false)}
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

        <Accordion disableGutters defaultExpanded>
          <AccordionSummary expandIcon={<span>▾</span>}>
            <Typography variant="body1">Plotting</Typography>
          </AccordionSummary>
          <AccordionDetails>
            <Stack spacing={2} sx={{ px: 1 }}>
              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  Plot Scale
                </Typography>
                <Slider
                  value={pipelineConfig.plotScale}
                  onChange={handleSliderChange("plotScale", true)}
                  min={1}
                  max={4}
                  step={1}
                  marks={marksPlotScale}
                  valueLabelDisplay="auto"
                  aria-label="Plot Scale"
                />
              </Box>

              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  Background Color
                </Typography>
                <RadioGroup
                  row
                  aria-label="Background Color"
                  name="background-color"
                  value={
                    pipelineConfig.backgroundColor ===
                    vektorModule.BackgroundColor.black
                      ? "black"
                      : "white"
                  }
                  onChange={(e: React.ChangeEvent<HTMLInputElement>) => {
                    const val = e.target.value;
                    setPipelineConfig((prev) => ({
                      ...prev,
                      backgroundColor:
                        val === "black"
                          ? vektorModule.BackgroundColor.black
                          : vektorModule.BackgroundColor.white,
                    }));
                  }}
                >
                  <FormControlLabel
                    value="black"
                    control={<Radio />}
                    label="Black"
                  />
                  <FormControlLabel
                    value="white"
                    control={<Radio />}
                    label="White"
                  />
                </RadioGroup>
              </Box>

              <Box>
                <Typography variant="subtitle2" gutterBottom>
                  Desmos Color
                </Typography>
                <RadioGroup
                  row
                  aria-label="Desmos Color"
                  name="desmos-color"
                  value={
                    pipelineConfig.desmosColor ===
                    vektorModule.DesmosColor.colorful
                      ? "colorful"
                      : "solid"
                  }
                  onChange={(e: React.ChangeEvent<HTMLInputElement>) => {
                    const val = e.target.value;
                    setPipelineConfig((prev) => ({
                      ...prev,
                      desmosColor:
                        val === "colorful"
                          ? vektorModule.DesmosColor.colorful
                          : vektorModule.DesmosColor.solid,
                    }));
                  }}
                >
                  <FormControlLabel
                    value="colorful"
                    control={<Radio />}
                    label="Colorful"
                  />
                  <FormControlLabel
                    value="solid"
                    control={<Radio />}
                    label="Solid"
                  />
                </RadioGroup>
              </Box>

              <Box
                sx={{
                  display: "flex",
                  justifyContent: "center",
                  alignItems: "center",
                  pt: 1,
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
