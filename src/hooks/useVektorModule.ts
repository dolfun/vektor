import { useState, useEffect } from "react";
import { MainModule } from "@/vektor";

export function useVektorModule(options?: unknown) {
  const [module, setModule] = useState<MainModule | null>(null);

  useEffect(() => {
    let cancelled = false;

    (async () => {
      const { default: initModule } = await import(
        /* @vite-ignore */ `${import.meta.env.BASE_URL}modules/vektor.mjs`
      );

      const instance = (await initModule(options)) as MainModule;

      if (!cancelled) setModule(instance);
    })();

    return () => {
      cancelled = true;
    };
  }, [options]);

  return module;
}
