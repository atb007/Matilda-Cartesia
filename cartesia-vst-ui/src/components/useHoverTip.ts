import { useCallback, useRef, useState } from "react";

/** Delayed hover tooltip — native `title` is unreliable over transformed layers. */
export function useHoverTip(delayMs = 400) {
  const [visible, setVisible] = useState(false);
  const timer = useRef(0);

  const onEnter = useCallback(() => {
    window.clearTimeout(timer.current);
    timer.current = window.setTimeout(() => setVisible(true), delayMs);
  }, [delayMs]);

  const onLeave = useCallback(() => {
    window.clearTimeout(timer.current);
    setVisible(false);
  }, []);

  return { visible, onEnter, onLeave };
}
