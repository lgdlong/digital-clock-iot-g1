import React from "react";

export default function AddAlarmModal({
  show,
  onClose,
  newTime,
  setNewTime,
  onAdd,
}: {
  show: boolean;
  onClose: () => void;
  newTime: string;
  setNewTime: (v: string) => void;
  onAdd: () => void;
}) {
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
        <div className="mb-3">
          <label className="form-label">Chọn giờ:</label>
          <input
            type="time"
            value={newTime}
            className="form-control bg-dark text-white"
            onChange={(e) => setNewTime(e.target.value)}
          />
        </div>
        <div className="d-flex gap-2 justify-content-center">
          <button className="btn btn-secondary" onClick={onClose}>
            Huỷ
          </button>
          <button className="btn btn-primary" onClick={onAdd}>
            Thêm
          </button>
        </div>
      </div>
    </div>
  );
}
