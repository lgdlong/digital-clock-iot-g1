// lib/alarms-api.ts
import type { Alarm } from "@/types/alarm.dto";

// Lấy danh sách báo thức
export async function getAlarms(): Promise<Alarm[]> {
  const res = await fetch("/api/alarms");
  if (!res.ok) throw new Error("Không lấy được danh sách báo thức");
  return await res.json();
}

// Thêm báo thức mới
export async function addAlarm(alarm: Omit<Alarm, "_id">): Promise<Alarm> {
  const res = await fetch("/api/set-alarm", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(alarm),
  });
  if (!res.ok) throw new Error("Không thêm được báo thức");
  return await res.json();
}

// Bật/tắt báo thức
export async function toggleAlarm(
  _id: string,
  enabled: boolean
): Promise<Alarm> {
  const res = await fetch("/api/update-alarm", {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ _id, enabled }),
  });
  if (!res.ok) throw new Error("Không cập nhật báo thức");
  return await res.json();
}

// Cập nhật báo thức
export async function updateAlarm(
  _id: string,
  updates: Partial<Omit<Alarm, "_id">>
): Promise<Alarm> {
  const res = await fetch("/api/update-alarm", {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ _id, ...updates }),
  });
  if (!res.ok) throw new Error("Không cập nhật được báo thức");
  return await res.json();
}

// Xoá báo thức
export async function deleteAlarm(_id: string): Promise<{ success: boolean }> {
  const res = await fetch(`/api/delete-alarm?_id=${_id}`, {
    method: "DELETE",
  });
  if (!res.ok) throw new Error("Không xoá được báo thức");
  return await res.json();
}
