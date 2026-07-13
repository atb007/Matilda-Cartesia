import { useEffect } from "react";
import {
  Alignment,
  Fit,
  Layout,
  useRive,
  useViewModel,
  useViewModelInstance,
  useViewModelInstanceBoolean,
} from "@rive-app/react-webgl2";
import {
  RIVE_ARTBOARD,
  RIVE_PLAY_BOOLEAN,
  RIVE_SRC,
  RIVE_VIEW_MODEL,
} from "../riveConfig";

type Props = {
  playing: boolean;
  className?: string;
  style?: React.CSSProperties;
};

/**
 * Rive Artboard hero — replaces static portrait when enableRiveHero() is on.
 * Boolean data bind streakVisible mirrors transport play/stop.
 */
export function MatildaRivePortrait({ playing, className, style }: Props) {
  const { RiveComponent, rive } = useRive(
    {
      src: RIVE_SRC,
      artboard: RIVE_ARTBOARD,
      autoplay: true,
      autoBind: false,
      layout: new Layout({
        fit: Fit.Cover,
        alignment: Alignment.CenterLeft,
      }),
    },
    { shouldResizeCanvasToContainer: true },
  );

  const viewModel = useViewModel(rive, { name: RIVE_VIEW_MODEL });
  const viewModelInstance = useViewModelInstance(viewModel, {
    useDefault: true,
    rive,
  });
  const { setValue: setStreakVisible } = useViewModelInstanceBoolean(
    RIVE_PLAY_BOOLEAN,
    viewModelInstance,
  );

  useEffect(() => {
    setStreakVisible(playing);
  }, [playing, setStreakVisible]);

  return (
    <div
      className={className}
      style={{
        position: "absolute",
        overflow: "hidden",
        pointerEvents: "none",
        ...style,
      }}
    >
      <RiveComponent
        style={{
          width: "100%",
          height: "100%",
          display: "block",
          background: "transparent",
        }}
      />
    </div>
  );
}
