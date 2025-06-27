"use client";
import { useState } from "react";
import AlarmList from "@/components/AlarmList";
import AddAlarmModal from "@/components/AddAlarmModal";

type Alarm = {
  id: number;
  datetime: string;
  enabled: boolean;
};

export default function AlarmPage() {
  const [alarms, setAlarms] = useState<Alarm[]>([
    { id: 1, datetime: "2024-06-29T06:30:00", enabled: true },
    { id: 2, datetime: "2024-06-29T21:15:00", enabled: false },
    { id: 3, datetime: "2024-06-29T07:15:00", enabled: true },
  ]);
  const [showAdd, setShowAdd] = useState(false);
  const [newTime, setNewTime] = useState("07:00");

  const toggleAlarm = (id: number) => {
    setAlarms((prev) =>
      prev.map((alarm) =>
        alarm.id === id ? { ...alarm, enabled: !alarm.enabled } : alarm
      )
    );
  };

  const deleteAlarm = (id: number) => {
    setAlarms((prev) => prev.filter((alarm) => alarm.id !== id));
  };

  const addAlarm = () => {
    setAlarms((prev) => [
      ...prev,
      {
        id: Date.now(),
        datetime: `2024-06-29T${newTime}:00`,
        enabled: true,
      },
    ]);
    setShowAdd(false);
    setNewTime("07:00");
  };

  return (
    <div className="container py-4" style={{ maxWidth: 420 }}>
      <h2 className="text-center mb-4">Báo thức</h2>
      <AlarmList
        alarms={alarms}
        onToggle={toggleAlarm}
        onDelete={deleteAlarm}
      />
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
        newTime={newTime}
        setNewTime={setNewTime}
        onAdd={addAlarm}
      />
    </div>
  );
}
