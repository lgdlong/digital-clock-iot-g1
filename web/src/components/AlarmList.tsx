// components/AlarmList.tsx
import { Alarm } from "@/types/alarm.dto";
import { useState } from "react";

const weekdays = ["CN", "T2", "T3", "T4", "T5", "T6", "T7"];

export default function AlarmList({
  alarms,
  onToggle,
  onDelete,
}: {
  alarms: Alarm[];
  onToggle: (id: string) => void;
  onDelete: (id: string) => void;
}) {
  const [pendingDelete, setPendingDelete] = useState<string | null>(null);

  const showTime = (h: number, m: number) =>
    `${h.toString().padStart(2, "0")}:${m.toString().padStart(2, "0")}`;

  const showDays = (ds: number[]) =>
    ds.length === 7 ? "Cả tuần" : ds.map((d) => weekdays[d]).join(", ");

  return (
    <ul className="list-group mb-4">
      {alarms.map((alarm) => (
        <li
          className="list-group-item d-flex align-items-center bg-dark text-white border-0 mb-2 shadow-sm"
          key={alarm._id}
          style={{
            borderRadius: 14,
            fontSize: 20,
            justifyContent: "space-between",
            padding: "14px 18px",
          }}
        >
          <div style={{ flex: 1, minWidth: 0 }}>
            <b>{showTime(alarm.hour, alarm.minute)}</b>
            <span style={{ fontSize: 13, color: "#aaa", marginLeft: 8 }}>
              {alarm.label && `· ${alarm.label}`}
              {alarm.enabled ? "" : " (Tắt)"}
            </span>
            <div style={{ fontSize: 13, color: "#cfcfcf", marginTop: 2 }}>
              {showDays(alarm.daysOfWeek)}
            </div>
          </div>
          <div className="d-flex align-items-center gap-2">
            <div className="form-check form-switch">
              <input
                className="form-check-input"
                type="checkbox"
                checked={alarm.enabled}
                onChange={() => onToggle(alarm._id)}
                style={{ width: 38, height: 22 }}
                aria-label={`${
                  alarm.enabled ? "Tắt" : "Bật"
                } báo thức ${showTime(alarm.hour, alarm.minute)}`}
                id={`alarm-toggle-${alarm._id}`}
              />
            </div>
            {pendingDelete === alarm._id ? (
              <button
                className="btn btn-sm btn-danger"
                style={{
                  borderRadius: "8px",
                  width: 80,
                  height: 32,
                  padding: 0,
                  fontSize: 15,
                }}
                onClick={() => {
                  setPendingDelete(null);
                  onDelete(alarm._id);
                }}
                title="Xác nhận xoá"
              >
                Xác nhận
              </button>
            ) : (
              <button
                className="btn btn-sm btn-outline-danger"
                style={{
                  borderRadius: "50%",
                  width: 32,
                  height: 32,
                  padding: 0,
                }}
                onClick={() => setPendingDelete(alarm._id)}
                title="Xoá"
              >
                &times;
              </button>
            )}
          </div>
        </li>
      ))}
      {alarms.length === 0 && (
        <li
          className="list-group-item bg-dark text-white border-0 text-center"
          style={{ borderRadius: 10 }}
        >
          Chưa có báo thức nào
        </li>
      )}
    </ul>
  );
}
