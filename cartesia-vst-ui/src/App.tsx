import { MatildaPluginFrame } from "./components/MatildaPluginFrame";

const FONT = "'Jost', sans-serif";

export default function App() {
  return (
    <main className="min-h-screen bg-[#0d0d0d] flex flex-col items-center justify-center gap-6 p-6 overflow-auto">
      <p
        className="text-[10px] text-white/20 tracking-widest uppercase select-none"
        style={{ fontFamily: FONT, letterSpacing: "0.2em" }}
      >
        M9 · engine-linked shell
      </p>

      <MatildaPluginFrame scale={0.52} />
    </main>
  );
}
