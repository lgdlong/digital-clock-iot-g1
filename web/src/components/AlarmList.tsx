import React, { useState } from "react";

type Alarm = {
  id: number;
  datetime: string; // ISO 8601
  enabled: boolean;
};

export default function AlarmList({
  alarms,
  onToggle,
  onDelete,
}: {
  alarms: Alarm[];
  onToggle: (id: number) => void;
  onDelete: (id: number) => void;
}) {
  const [pendingDelete, setPendingDelete] = useState<number | null>(null);

  const showTime = (dt: string) => {
    try {
      return new Date(dt).toLocaleTimeString("vi-VN", {
        hour: "2-digit",
        minute: "2-digit",
        hour12: false,
      });
    } catch (error) {
      console.error("Error parsing datetime:", dt, error);
      return "--:--";
    }
  };

  return (
    <ul className="list-group mb-4">
      {alarms.map((alarm) => (
        <li
          className="list-group-item d-flex align-items-center bg-dark text-white border-0 mb-2 shadow-sm"
          key={alarm.id}
          style={{
            borderRadius: 14,
            fontSize: 22,
            justifyContent: "space-between",
            padding: "16px 20px",
          }}
        >
          <span>
            <b>{showTime(alarm.datetime)}</b>
            <span style={{ fontSize: 12, color: "#aaa", marginLeft: 8 }}>
              {alarm.enabled ? "" : " (Tắt)"}
            </span>
          </span>
          <div className="d-flex align-items-center gap-2">
            <div className="form-check form-switch">
              <input
                className="form-check-input"
                type="checkbox"
                checked={alarm.enabled}
                onChange={() => onToggle(alarm.id)}
                style={{ width: 38, height: 22 }}
                aria-label={`${
                  alarm.enabled ? "Tắt" : "Bật"
                } báo thức ${showTime(alarm.datetime)}`}
                id={`alarm-toggle-${alarm.id}`}
              />
            </div>
            {pendingDelete === alarm.id ? (
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
                  onDelete(alarm.id);
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
                onClick={() => setPendingDelete(alarm.id)}
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
