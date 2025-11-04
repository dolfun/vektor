import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  base: "/vektor/",
  plugins: [react()],
  resolve: {
    alias: {
      "@": "/src",
    },
  },
});
