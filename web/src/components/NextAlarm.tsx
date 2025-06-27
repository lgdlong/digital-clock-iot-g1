// components/NextAlarm.tsx
export default function NextAlarm({ alarmTime }: { alarmTime: string }) {
  return (
    <div className="mb-3 text-center">
      <span className="badge bg-info text-dark" style={{ fontSize: 16 }}>
        Báo thức sắp tới: {alarmTime}
      </span>
    </div>
  );
}
