"use client";
import React from "react";

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
  // Hiển thị giờ đẹp
  const showTime = (dt: string) => dt.slice(11, 16);

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
              />
            </div>
            <button
              className="btn btn-sm btn-outline-danger"
              style={{ borderRadius: "50%", width: 32, height: 32, padding: 0 }}
              onClick={() => {
                if (window.confirm('Bạn có chắc muốn xóa báo thức này?')) {
                  onDelete(alarm.id);
                }
              }}
              title="Xoá"
            >
              &times;
            </button>
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
