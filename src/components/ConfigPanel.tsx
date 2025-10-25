import {
  Accordion,
  AccordionDetails,
  AccordionSummary,
  Box,
  Checkbox,
  Divider,
  FormControlLabel,
  Paper,
  Stack,
  Typography,
} from "@mui/material";

export function ConfigPanel({
  showFinal,
  onToggleFinal,
}: {
  showFinal: boolean;
  onToggleFinal: (v: boolean) => void;
}) {
  const stageNames = [1, 2].map((i) => `Stage ${i}`);

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
        {stageNames.map((name, idx) => (
          <Accordion key={idx} disableGutters>
            <AccordionSummary expandIcon={<span>▾</span>}>
              <Typography variant="body1">{name}</Typography>
            </AccordionSummary>
            <AccordionDetails>
              <Typography variant="body2" color="text.secondary">
                No options yet.
              </Typography>
            </AccordionDetails>
          </Accordion>
        ))}
        <Accordion disableGutters>
          <AccordionSummary expandIcon={<span>▾</span>}>
            <Typography variant="body1">Final Result</Typography>
          </AccordionSummary>
          <AccordionDetails>
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
                label={<Typography variant="subtitle2">Show</Typography>}
              />
            </Box>
          </AccordionDetails>
        </Accordion>
      </Stack>
    </Paper>
  );
}
