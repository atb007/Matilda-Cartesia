import { useEffect, type RefObject } from "react";

/** Close menus when the user taps/clicks outside `ref` (trigger + popup). */
export function useClickOutside(
  ref: RefObject<HTMLElement | null>,
  active: boolean,
  onDismiss: () => void,
) {
  useEffect(() => {
    if (!active) return;
    const handler = (e: PointerEvent) => {
      const el = ref.current;
      if (el && !el.contains(e.target as Node)) onDismiss();
    };
    document.addEventListener("pointerdown", handler);
    return () => document.removeEventListener("pointerdown", handler);
  }, [active, onDismiss, ref]);
}
