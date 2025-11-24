import React, { createContext, useContext, useState, useEffect } from "react";
import { VektorModule } from "@/vektor";

const VektorModuleContext = createContext<VektorModule | null>(null);

export function VektorModuleProvider({
  children,
}: {
  children: React.ReactNode;
}) {
  const [module, setModule] = useState<VektorModule | null>(null);

  useEffect(() => {
    let cancelled = false;

    (async () => {
      const { default: initModule } = await import(
        /* @vite-ignore */ `${import.meta.env.BASE_URL}modules/vektor.mjs`
      );

      const instance = (await initModule()) as VektorModule;
      if (!cancelled) setModule(instance);
    })();

    return () => {
      cancelled = true;
    };
  }, []);

  if (!module) {
    return null;
  }

  return (
    <VektorModuleContext.Provider value={module}>
      {children}
    </VektorModuleContext.Provider>
  );
}

export function useVektorModule(): VektorModule {
  const module = useContext(VektorModuleContext);
  if (!module) {
    throw new Error(
      "useVektorModule must be used inside a VektorModuleProvider *after* the module has loaded."
    );
  }

  return module;
}
