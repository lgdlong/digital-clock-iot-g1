"use client";
import ClockDisplay from "@/components/ClockDisplay";
import DeviceStatus from "@/components/DeviceStatus";
import NextAlarm from "@/components/NextAlarm";
import Link from "next/link";

export default function Home() {
  // Data mock
  const now = "09:26:41";
  const isOnline = true;
  const nextAlarm = "06:30";

  return (
    <div className="container py-5">
      <ClockDisplay time={now} />
      <DeviceStatus online={isOnline} />
      <NextAlarm alarmTime={nextAlarm} />
      <div className="my-3 text-center">
        <Link href="/alarm" className="btn btn-primary">
          + Thêm báo thức
        </Link>
      </div>
    </div>
  );
}
