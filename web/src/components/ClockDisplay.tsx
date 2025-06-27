// components/ClockDisplay.tsx
import "@/css/clock-display.css";

export default function ClockDisplay({ time }: { time: string }) {
  return (
    <div className="text-center mb-4">
      <div className="clock-display">{time}</div>
    </div>
  );
}
