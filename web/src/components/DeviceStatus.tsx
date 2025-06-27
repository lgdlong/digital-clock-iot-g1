// components/DeviceStatus.tsx
export default function DeviceStatus({ online }: { online: boolean }) {
  return (
    <div className="mb-3 text-center">
      <span
        className={`badge ${online ? "bg-success" : "bg-danger"} me-2`}
        style={{ fontSize: 16 }}
      >
        {online ? "Thiết bị Online" : "Thiết bị Offline"}
      </span>
    </div>
  );
}
