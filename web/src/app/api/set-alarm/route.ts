// app/api/set-alarm/route.ts
import { NextRequest, NextResponse } from "next/server";
import { connectDB } from "@/config/db-config";
import type { Alarm } from "@/types/alarm.dto";

interface CreateAlarmRequest {
  hour: number;
  minute: number;
  daysOfWeek: number[];
  enabled: boolean;
  label?: string;
}

function validateAlarmData(data: unknown): data is CreateAlarmRequest {
  if (typeof data !== "object" || data === null) return false;

  const obj = data as Record<string, unknown>;

  return (
    typeof obj.hour === "number" &&
    obj.hour >= 0 &&
    obj.hour <= 23 &&
    typeof obj.minute === "number" &&
    obj.minute >= 0 &&
    obj.minute <= 59 &&
    Array.isArray(obj.daysOfWeek) &&
    obj.daysOfWeek.every(
      (day: unknown) => typeof day === "number" && day >= 0 && day <= 6
    ) &&
    typeof obj.enabled === "boolean" &&
    (obj.label === undefined || typeof obj.label === "string")
  );
}

/**
 * @swagger
 * /api/set-alarm:
 *   post:
 *     summary: Create a new alarm
 *     description: Create a new alarm with specified time, days, and settings
 *     tags:
 *       - Alarms
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/AlarmCreate'
 *           example:
 *             hour: 7
 *             minute: 30
 *             daysOfWeek: [1, 2, 3, 4, 5]
 *             enabled: true
 *             label: "Wake up"
 *     responses:
 *       201:
 *         description: Alarm created successfully
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Alarm'
 *       400:
 *         description: Invalid alarm data
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       500:
 *         description: Internal server error
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 */
export async function POST(request: NextRequest) {
  try {
    const body = await request.json();

    // Validate request data
    if (!validateAlarmData(body)) {
      return NextResponse.json(
        {
          error:
            "Invalid alarm data. Required fields: hour (0-23), minute (0-59), daysOfWeek (array of 0-6), enabled (boolean), optional label (string)",
        },
        { status: 400 }
      );
    }

    const db = await connectDB();

    // Create alarm document without _id (MongoDB will generate it)
    const alarmToInsert = {
      hour: body.hour,
      minute: body.minute,
      daysOfWeek: body.daysOfWeek,
      enabled: body.enabled,
      label: body.label || "",
    };

    const result = await db.collection("alarms").insertOne(alarmToInsert);

    if (!result.acknowledged) {
      return NextResponse.json(
        { error: "Failed to create alarm" },
        { status: 500 }
      );
    }

    // Return the created alarm with _id as string
    const createdAlarm: Alarm = {
      ...alarmToInsert,
      _id: result.insertedId.toString(),
    };

    return NextResponse.json(createdAlarm, { status: 201 });
  } catch (error) {
    console.error("Error creating alarm:", error);
    return NextResponse.json(
      { error: "Failed to create alarm" },
      { status: 500 }
    );
  }
}
