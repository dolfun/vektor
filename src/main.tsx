import React from "react";
import ReactDOM from "react-dom/client";
import CssBaseline from "@mui/material/CssBaseline";
import { ThemeProvider, createTheme } from "@mui/material/styles";

import { useVektorModule } from "@/hooks";
import App from "@/App";

const darkTheme = createTheme({
  palette: { mode: "dark" },
});

function Main() {
  const vektorModule = useVektorModule();

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      {vektorModule && <App vektorModule={vektorModule} />}
    </ThemeProvider>
  );
}

ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <Main />
  </React.StrictMode>
);
