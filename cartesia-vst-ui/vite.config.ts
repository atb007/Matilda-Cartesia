import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import tailwindcss from "@tailwindcss/vite";
import { readFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const rootDir = path.dirname(fileURLToPath(import.meta.url));
const pluginVersion = readFileSync(
  path.resolve(rootDir, "../matilda/plugin/VERSION"),
  "utf8",
).trim();

// https://vite.dev/config/
export default defineConfig({
  plugins: [react(), tailwindcss()],
  define: {
    __MATILDA_VERSION__: JSON.stringify(pluginVersion),
  },
});
