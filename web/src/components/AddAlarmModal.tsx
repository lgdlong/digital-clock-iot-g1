import React, { useState, useEffect } from "react";

// days: 0=CN, 1=Thứ 2, ..., 6=Thứ 7
const weekdays = [
  { label: "CN", value: 0 },
  { label: "T2", value: 1 },
  { label: "T3", value: 2 },
  { label: "T4", value: 3 },
  { label: "T5", value: 4 },
  { label: "T6", value: 5 },
  { label: "T7", value: 6 },
];

export default function AddAlarmModal({
  show,
  onClose,
  newHour,
  setNewHour,
  newMinute,
  setNewMinute,
  newDays,
  setNewDays,
  newLabel,
  setNewLabel,
  onAdd,
}: {
  show: boolean;
  onClose: () => void;
  newHour: number;
  setNewHour: (v: number) => void;
  newMinute: number;
  setNewMinute: (v: number) => void;
  newDays: number[];
  setNewDays: (v: number[]) => void;
  newLabel: string;
  setNewLabel: (v: string) => void;
  onAdd: () => void;
}) {
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (!show) setError(null); // reset lỗi khi đóng modal
  }, [show]);

  // Toggle ngày trong tuần
  const toggleDay = (d: number) => {
    if (newDays.includes(d)) setNewDays(newDays.filter((x) => x !== d));
    else setNewDays([...newDays, d].sort());
  };

  const handleAdd = () => {
    // Validate dữ liệu
    if (
      isNaN(newHour) ||
      isNaN(newMinute) ||
      newHour < 0 ||
      newHour > 23 ||
      newMinute < 0 ||
      newMinute > 59
    ) {
      setError("Vui lòng nhập giờ và phút hợp lệ.");
      return;
    }
    if (!newDays.length) {
      setError("Phải chọn ít nhất một ngày trong tuần.");
      return;
    }
    setError(null);
    onAdd();
  };

  if (!show) return null;

  return (
    <div
      style={{
        position: "fixed",
        top: 0,
        left: 0,
        zIndex: 1000,
        width: "100vw",
        height: "100vh",
        background: "#000a",
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
      }}
      onClick={onClose}
    >
      <div
        className="card bg-secondary text-white p-4"
        style={{ minWidth: 320, maxWidth: 340 }}
        onClick={(e) => e.stopPropagation()}
      >
        <h5 className="mb-3 text-center">Thêm báo thức mới</h5>
        <div className="mb-3 d-flex gap-2">
          <div>
            <label className="form-label">Giờ:</label>
            <input
              type="number"
              min={0}
              max={23}
              className="form-control bg-dark text-white"
              value={newHour}
              onChange={(e) => setNewHour(Number(e.target.value))}
            />
          </div>
          <div>
            <label className="form-label">Phút:</label>
            <input
              type="number"
              min={0}
              max={59}
              className="form-control bg-dark text-white"
              value={newMinute}
              onChange={(e) => setNewMinute(Number(e.target.value))}
            />
          </div>
        </div>
        <div className="mb-3">
          <label className="form-label">Chọn ngày:</label>
          <div className="d-flex gap-1 flex-wrap">
            {weekdays.map((day) => (
              <button
                type="button"
                key={day.value}
                className={`btn btn-sm ${
                  newDays.includes(day.value)
                    ? "btn-primary"
                    : "btn-outline-light"
                }`}
                style={{ minWidth: 36, fontWeight: "bold" }}
                onClick={() => toggleDay(day.value)}
              >
                {day.label}
              </button>
            ))}
          </div>
        </div>
        <div className="mb-3">
          <label className="form-label">Ghi chú:</label>
          <input
            type="text"
            value={newLabel}
            maxLength={30}
            className="form-control bg-dark text-white"
            placeholder="Nhập tên báo thức (tùy chọn)"
            onChange={(e) => setNewLabel(e.target.value)}
            autoComplete="off"
          />
        </div>
        {error && (
          <div className="alert alert-danger py-2 text-center mb-2">
            {error}
          </div>
        )}
        <div className="d-flex gap-2 justify-content-center">
          <button className="btn btn-secondary" onClick={onClose}>
            Huỷ
          </button>
          <button className="btn btn-primary" onClick={handleAdd}>
            Thêm
          </button>
        </div>
      </div>
    </div>
  );
}
