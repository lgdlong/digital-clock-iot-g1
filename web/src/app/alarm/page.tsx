"use client";
import { useEffect, useState } from "react";
import AlarmList from "@/components/AlarmList";
import AddAlarmModal from "@/components/AddAlarmModal";
import type { Alarm } from "@/types/alarm.dto";
import {
  getAlarms,
  addAlarm,
  toggleAlarm as toggleAlarmAPI,
  deleteAlarm as deleteAlarmAPI,
} from "@/lib/alarms-api";

export default function AlarmPage() {
  const [alarms, setAlarms] = useState<Alarm[]>([]);
  const [showAdd, setShowAdd] = useState(false);
  const [newHour, setNewHour] = useState(7);
  const [newMinute, setNewMinute] = useState(0);
  const [newDays, setNewDays] = useState<number[]>([new Date().getDay() || 0]); // Ngày trong tuần hiện tại
  const [newLabel, setNewLabel] = useState("Alarm");
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    setLoading(true);
    setError(null);
    getAlarms()
      .then(setAlarms)
      .catch((err: unknown) => {
        if (err instanceof Error) {
          setError(err.message);
        } else {
          setError("Đã xảy ra lỗi không xác định");
        }
      })
      .finally(() => setLoading(false));
  }, []);

  const handleToggle = async (_id: string) => {
    setAlarms((prev) =>
      prev.map((alarm) =>
        alarm._id === _id ? { ...alarm, enabled: !alarm.enabled } : alarm
      )
    );
    try {
      const alarm = alarms.find((a) => a._id === _id);
      if (alarm) await toggleAlarmAPI(_id, !alarm.enabled);
    } catch (err) {
      setError("Không thể cập nhật báo thức");
      console.error(err);
      // rollback nếu cần
    }
  };

  const handleDelete = async (_id: string) => {
    setAlarms((prev) => prev.filter((alarm) => alarm._id !== _id));
    try {
      await deleteAlarmAPI(_id);
    } catch (err) {
      setError("Không thể xoá báo thức");
      console.error(err);
      // rollback nếu cần
    }
  };

  const handleAdd = async () => {
    try {
      const alarm = await addAlarm({
        hour: newHour,
        minute: newMinute,
        enabled: true,
        daysOfWeek: [...newDays],
        label: newLabel,
      });
      setAlarms((prev) => [...prev, alarm]);
      setShowAdd(false);
      setNewHour(7);
      setNewMinute(0);
      setNewDays([new Date().getDay() || 0]);
      setNewLabel("");
    } catch (err) {
      setError("Không thêm được báo thức");
      console.error(err);
    }
  };

  return (
    <div className="container py-4" style={{ maxWidth: 420 }}>
      <h2 className="text-center mb-4">Báo thức</h2>
      {loading ? (
        <div className="text-center my-4">Đang tải...</div>
      ) : error ? (
        <div className="alert alert-danger text-center">{error}</div>
      ) : (
        <AlarmList
          alarms={alarms}
          onToggle={handleToggle}
          onDelete={handleDelete}
        />
      )}
      <div className="text-center">
        <button
          className="btn btn-primary btn-lg rounded-circle"
          style={{
            width: 56,
            height: 56,
            fontSize: 32,
            lineHeight: "32px",
            boxShadow: "0 4px 16px #0006",
          }}
          onClick={() => setShowAdd(true)}
          title="Thêm báo thức"
        >
          +
        </button>
      </div>
      <AddAlarmModal
        show={showAdd}
        onClose={() => setShowAdd(false)}
        newHour={newHour}
        setNewHour={setNewHour}
        newMinute={newMinute}
        setNewMinute={setNewMinute}
        newDays={newDays}
        setNewDays={setNewDays}
        newLabel={newLabel}
        setNewLabel={setNewLabel}
        onAdd={handleAdd}
      />
    </div>
  );
}
