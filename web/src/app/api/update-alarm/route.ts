// app/api/update-alarm/route.ts
import { NextRequest, NextResponse } from "next/server";
import { connectDB } from "@/config/db-config";
import { ObjectId } from "mongodb";
import type { Alarm } from "@/types/alarm.dto";

interface UpdateAlarmRequest {
  _id: string;
  hour?: number;
  minute?: number;
  daysOfWeek?: number[];
  enabled?: boolean;
  label?: string;
}

function validateUpdateAlarmData(data: unknown): data is UpdateAlarmRequest {
  if (typeof data !== "object" || data === null) return false;

  const obj = data as Record<string, unknown>;

  return (
    typeof obj._id === "string" &&
    obj._id.length > 0 &&
    (obj.hour === undefined ||
      (typeof obj.hour === "number" && obj.hour >= 0 && obj.hour <= 23)) &&
    (obj.minute === undefined ||
      (typeof obj.minute === "number" &&
        obj.minute >= 0 &&
        obj.minute <= 59)) &&
    (obj.daysOfWeek === undefined ||
      (Array.isArray(obj.daysOfWeek) &&
        obj.daysOfWeek.every(
          (day: unknown) => typeof day === "number" && day >= 0 && day <= 6
        ))) &&
    (obj.enabled === undefined || typeof obj.enabled === "boolean") &&
    (obj.label === undefined || typeof obj.label === "string")
  );
}

export async function PATCH(request: NextRequest) {
  try {
    const body = await request.json();

    // Validate request data
    if (!validateUpdateAlarmData(body)) {
      return NextResponse.json(
        {
          error:
            "Invalid update data. Required: _id (string). Optional: hour (0-23), minute (0-59), daysOfWeek (array of 0-6), enabled (boolean), label (string)",
        },
        { status: 400 }
      );
    }

    const db = await connectDB();

    // Validate ObjectId format
    let objectId: ObjectId;
    try {
      objectId = new ObjectId(body._id);
    } catch {
      return NextResponse.json(
        { error: "Invalid alarm ID format" },
        { status: 400 }
      );
    }

    // Prepare update object (remove _id from update fields)
    const updateFields: Partial<Omit<Alarm, "_id">> = {
      hour: body.hour,
      minute: body.minute,
      daysOfWeek: body.daysOfWeek,
      enabled: body.enabled,
      label: body.label,
    };

    // Only update fields that are provided
    const updateDoc: Record<string, unknown> = {};
    if (updateFields.hour !== undefined) updateDoc.hour = updateFields.hour;
    if (updateFields.minute !== undefined)
      updateDoc.minute = updateFields.minute;
    if (updateFields.daysOfWeek !== undefined)
      updateDoc.daysOfWeek = updateFields.daysOfWeek;
    if (updateFields.enabled !== undefined)
      updateDoc.enabled = updateFields.enabled;
    if (updateFields.label !== undefined) updateDoc.label = updateFields.label;

    if (Object.keys(updateDoc).length === 0) {
      return NextResponse.json(
        { error: "No valid fields to update" },
        { status: 400 }
      );
    }

    const result = await db
      .collection("alarms")
      .findOneAndUpdate(
        { _id: objectId },
        { $set: updateDoc },
        { returnDocument: "after" }
      );

    if (!result.value) {
      return NextResponse.json({ error: "Alarm not found" }, { status: 404 });
    }

    // Return the updated alarm with _id as string
    const updatedAlarm: Alarm = {
      hour: result.value.hour,
      minute: result.value.minute,
      enabled: result.value.enabled,
      daysOfWeek: result.value.daysOfWeek,
      label: result.value.label,
      _id: result.value._id.toString(),
    };

    return NextResponse.json(updatedAlarm);
  } catch (error) {
    console.error("Error updating alarm:", error);
    return NextResponse.json(
      { error: "Failed to update alarm" },
      { status: 500 }
    );
  }
}
