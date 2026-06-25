/** Injected from matilda/plugin/VERSION via vite.config.ts */
declare const __MATILDA_VERSION__: string;

export const PLUGIN_VERSION = __MATILDA_VERSION__;

export function cartesiaVersionLabel(): string {
  return `Cartesia - v${PLUGIN_VERSION}`;
}
