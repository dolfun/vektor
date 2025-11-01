import {
  useState,
  useRef,
  useEffect,
  useLayoutEffect,
  useCallback,
} from "react";

import {
  Box,
  Button,
  Card,
  Checkbox,
  FormControlLabel,
  IconButton,
  Slider,
  Stack,
  Typography,
} from "@mui/material";
import { ArrowBack, ArrowForward } from "@mui/icons-material";

import type { Stage } from "@/utility";

export function MultiStageCanvas({ stages }: { stages: Stage[] }) {
  const [index, setIndex] = useState(0);
  const stage = stages[index];
  const prev = () => setIndex((i) => (i - 1 + stages.length) % stages.length);
  const next = () => setIndex((i) => (i + 1) % stages.length);

  const wrapRef = useRef<HTMLDivElement | null>(null);
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const offscreenRef = useRef<HTMLCanvasElement | null>(null);

  const [fit, setFit] = useState(1);
  const [scale, setScale] = useState(1);
  const [tx, setTx] = useState(0);
  const [ty, setTy] = useState(0);
  const [pixelated, setPixelated] = useState(false);
  const [drag, setDrag] = useState({
    on: false,
    x: 0,
    y: 0,
    tx0: 0,
    ty0: 0,
  });

  const refreshOffscreen = useCallback(() => {
    const {
      imageData: { data, width, height },
    } = stage;
    let off = offscreenRef.current;
    if (!off) {
      off = document.createElement("canvas");
      offscreenRef.current = off;
    }
    off.width = width;
    off.height = height;
    const g = off.getContext("2d")!;
    const img = g.createImageData(width, height);
    img.data.set(data);
    g.putImageData(img, 0, 0);
  }, [stage]);

  const fitAndCenter = useCallback(() => {
    const wrap = wrapRef.current;
    if (!wrap) return;
    const { clientWidth: cw, clientHeight: ch } = wrap;
    if (!cw || !ch) return;
    const { width, height } = stage.imageData;
    const s = Math.min(cw / width, ch / height);
    const cx = (cw - width * s) / 2;
    const cy = (ch - height * s) / 2;

    setFit(s);
    setScale(s);
    setTx(cx);
    setTy(cy);
  }, [stage]);

  useLayoutEffect(() => {
    refreshOffscreen();
    fitAndCenter();
  }, [stage, refreshOffscreen, fitAndCenter]);

  useLayoutEffect(() => {
    const wrap = wrapRef.current;
    if (!wrap) return;
    const ro = new ResizeObserver(fitAndCenter);
    ro.observe(wrap);
    return () => ro.disconnect();
  }, [fitAndCenter]);

  useLayoutEffect(() => {
    const wrap = wrapRef.current,
      canvas = canvasRef.current,
      off = offscreenRef.current;
    if (!wrap || !canvas || !off) return;

    canvas.width = wrap.clientWidth;
    canvas.height = wrap.clientHeight;

    const ctx = canvas.getContext("2d")!;
    ctx.imageSmoothingEnabled = !pixelated;
    ctx.imageSmoothingQuality = pixelated ? "low" : "high";

    ctx.setTransform(scale, 0, 0, scale, tx, ty);
    ctx.clearRect(
      -tx / scale,
      -ty / scale,
      canvas.width / scale,
      canvas.height / scale
    );
    ctx.drawImage(off, 0, 0);
  }, [scale, tx, ty, pixelated, stage]);

  const zoomTo = (v: number) => {
    const wrap = wrapRef.current;
    if (!wrap) return;
    const cx = wrap.clientWidth / 2,
      cy = wrap.clientHeight / 2;
    const clamped = Math.min(8, Math.max(v, fit));
    const k = clamped / scale;
    setScale(clamped);
    setTx(cx - k * (cx - tx));
    setTy(cy - k * (cy - ty));
  };

  const onPointerDown: React.PointerEventHandler<HTMLCanvasElement> = (e) => {
    (e.currentTarget as any).setPointerCapture?.(e.pointerId);
    setDrag({ on: true, x: e.clientX, y: e.clientY, tx0: tx, ty0: ty });
  };
  const onPointerMove: React.PointerEventHandler<HTMLCanvasElement> = (e) => {
    if (!drag.on) return;
    setTx(drag.tx0 + (e.clientX - drag.x));
    setTy(drag.ty0 + (e.clientY - drag.y));
  };
  const endDrag = () => setDrag((d) => ({ ...d, on: false }));

  const resetView = () => fitAndCenter();

  return (
    <Stack sx={{ height: "100%", minWidth: 0 }}>
      <Box sx={{ flex: 1, minHeight: 0 }}>
        <Stack sx={{ height: "100%" }}>
          <Stack
            direction="row"
            alignItems="center"
            justifyContent="center"
            spacing={2}
            sx={{ px: 1 }}
          >
            <Typography variant="body2">Zoom</Typography>
            <Slider
              value={scale}
              min={fit}
              max={8}
              step={0.05}
              onChange={(_, v) => zoomTo(v as number)}
              sx={{ width: 200 }}
            />
            <Button size="small" variant="outlined" onClick={resetView}>
              Reset
            </Button>
            <FormControlLabel
              control={
                <Checkbox
                  checked={pixelated}
                  onChange={(e) => setPixelated(e.target.checked)}
                />
              }
              label="Pixelated"
            />
          </Stack>

          <Card variant="outlined" sx={{ flex: 1, minHeight: 0 }}>
            <div
              ref={wrapRef}
              style={{ position: "relative", width: "100%", height: "100%" }}
            >
              <canvas
                ref={canvasRef}
                onPointerDown={onPointerDown}
                onPointerMove={onPointerMove}
                onPointerUp={endDrag}
                onPointerCancel={endDrag}
                style={{
                  width: "100%",
                  height: "100%",
                  display: "block",
                  cursor: drag.on ? "grabbing" : "grab",
                  imageRendering: pixelated ? "pixelated" : "auto",
                }}
              />
            </div>
          </Card>
        </Stack>
      </Box>

      <Box
        sx={{
          display: "flex",
          justifyContent: "center",
          alignItems: "center",
          gap: 1,
          py: 1,
        }}
      >
        <IconButton color="primary" onClick={prev} aria-label="Previous stage">
          <ArrowBack />
        </IconButton>
        <Typography
          variant="subtitle1"
          sx={{
            textAlign: "center",
            whiteSpace: "nowrap",
            overflow: "hidden",
            textOverflow: "ellipsis",
            px: 1,
            minWidth: 120,
          }}
          title={stage.stageName}
        >
          {stage.stageName}
        </Typography>
        <IconButton color="primary" onClick={next} aria-label="Next stage">
          <ArrowForward />
        </IconButton>
      </Box>
    </Stack>
  );
}
