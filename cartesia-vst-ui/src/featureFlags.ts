/**
 * Rive hero — browser dev testing only (`npm run dev:standalone-mac-rive`).
 * The macOS Standalone .app keeps the static portrait (WKWebView broke layout/resize).
 */
export function enableRiveHero(): boolean {
  return import.meta.env.VITE_STANDALONE_MAC_RIVE === "true";
}
