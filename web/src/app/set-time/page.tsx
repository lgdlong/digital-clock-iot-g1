"use client";
import { useEffect, useState } from "react";
import { DateTime } from "luxon";
import { timeZonesNames } from "@vvo/tzdb";

type TimezoneOption = string;

async function fetchDeviceTime(timezone?: string) {
  let url = "/api/get-device-time";
  if (timezone) url += "?timezone=" + encodeURIComponent(timezone);
  const res = await fetch(url);
  if (!res.ok) throw new Error("Lỗi kết nối thiết bị");
  const data = await res.json();
  return data.datetime as string;
}

async function setDeviceTimezone(timezone: string) {
  const res = await fetch("/api/set-device-timezone", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ timezone }),
  });
  if (!res.ok) throw new Error("Không cập nhật được múi giờ");
  return await res.json();
}

export default function SetTimePage() {
  const [deviceTime, setDeviceTime] = useState<string>("--:--:--");
  const [timezone, setTimezone] = useState<TimezoneOption>(
    Intl.DateTimeFormat().resolvedOptions().timeZone
  );
  const [loading, setLoading] = useState(false);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const refreshTime = async (tz: string = timezone) => {
    setLoading(true);
    setError(null);
    try {
      const iso = await fetchDeviceTime(tz);
      const dt = DateTime.fromISO(iso).setZone(tz);
      setDeviceTime(dt.toFormat("HH:mm:ss - dd/MM/yyyy"));
    } catch (err) {
      console.error("Error fetching device time:", err);
      setError("Không lấy được giờ từ thiết bị.");
    } finally {
      setLoading(false);
    }
  };

  const handleTimezoneChange = async (
    e: React.ChangeEvent<HTMLSelectElement>
  ) => {
    const tz = e.target.value;
    setTimezone(tz);
    setSaving(true);
    setError(null);
    try {
      await setDeviceTimezone(tz);
      await refreshTime(tz);
    } catch (err) {
      console.error("Error setting device timezone:", err);
      setError("Không cập nhật được múi giờ.");
    } finally {
      setSaving(false);
    }
  };

  useEffect(() => {
    refreshTime();
    // eslint-disable-next-line
  }, []);

  return (
    <div className="container py-5" style={{ maxWidth: 400 }}>
      <h2 className="mb-3 text-center">Giờ hiện tại trên thiết bị</h2>
      <div
        style={{
          fontSize: 32,
          fontWeight: "bold",
          letterSpacing: 2,
          textAlign: "center",
          minHeight: 48,
        }}
      >
        {loading ? "Đang lấy giờ..." : deviceTime}
      </div>
      <div className="mb-3 mt-4">
        <label className="form-label">Múi giờ hiện tại:</label>
        <select
          className="form-select"
          value={timezone}
          onChange={handleTimezoneChange}
          disabled={saving}
          style={{ maxWidth: 300 }}
        >
          {timeZonesNames.map((tz) => (
            <option value={tz} key={tz}>
              {tz}
            </option>
          ))}
        </select>
      </div>
      <div className="mt-2 mb-4 text-center" style={{ color: "#a1a1aa" }}>
        Đồng hồ tự động đồng bộ giờ chuẩn khi có WiFi.
        <br />
        Khi đổi múi giờ, đồng hồ sẽ cập nhật lại toàn bộ giờ hệ thống và báo
        thức.
      </div>
      <div className="text-center">
        <button
          onClick={() => refreshTime()}
          className="btn btn-outline-light btn-sm"
          disabled={loading || saving}
        >
          Làm mới giờ
        </button>
      </div>
      {saving && (
        <div className="text-info mt-2 text-center">
          Đang cập nhật múi giờ...
        </div>
      )}
      {error && (
        <div className="alert alert-danger mt-3 text-center">{error}</div>
      )}
    </div>
  );
}
