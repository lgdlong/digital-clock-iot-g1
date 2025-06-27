"use client";
export default function SettingsPage() {
  return (
    <div className="container py-4" style={{ maxWidth: 420 }}>
      <h2 className="text-center mb-4">Cài đặt thiết bị</h2>
      <div className="mb-3">
        <label className="form-label">Tên thiết bị</label>
        <input className="form-control" defaultValue="Default" />
        <button className="btn btn-primary btn-sm mt-2">Lưu tên</button>
      </div>
      <div className="mb-3">
        <label className="form-label">Múi giờ</label>
        <div>
          {/* Gắn component chọn múi giờ ở đây hoặc link sang set-time */}
          <a href="/set-time" className="btn btn-outline-secondary btn-sm">
            Chỉnh múi giờ
          </a>
        </div>
      </div>
      <div className="mb-3">
        <label className="form-label">ID Thiết bị</label>
        <input className="form-control" value="esp32-abc123" readOnly />
      </div>
      <div className="mb-3">
        <label className="form-label">WiFi hiện tại</label>
        <input className="form-control" value="LGD-HOME" readOnly />
      </div>
      <div className="mb-4">
        <button
          className="btn btn-danger"
          onClick={() =>
            confirm("Reset lại thiết bị?") && alert("Đã gửi lệnh reset!")
          }
        >
          Đặt lại thiết bị
        </button>
      </div>
      <div className="text-secondary text-end">Phiên bản firmware: 1.0.0</div>
    </div>
  );
}
