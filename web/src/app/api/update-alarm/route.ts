// app/api/update-alarm/route.ts
import { NextRequest, NextResponse } from "next/server";
import { connectDB } from "@/config/db-config";
import { ObjectId } from "mongodb";
import type { Alarm } from "@/types/alarm.dto";

// Interface định nghĩa dữ liệu yêu cầu cập nhật báo thức
interface UpdateAlarmRequest {
  _id: string;
  hour?: number;
  minute?: number;
  daysOfWeek?: number[];
  enabled?: boolean;
  label?: string;
}

// Hàm kiểm tra tính hợp lệ của dữ liệu đầu vào
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

/**
 * @swagger
 * /api/update-alarm:
 *   patch:
 *     summary: Update an existing alarm
 *     description: Update specific fields of an existing alarm. Only provided fields will be updated.
 *     tags:
 *       - Alarms
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required:
 *               - _id
 *             properties:
 *               _id:
 *                 type: string
 *                 description: Unique identifier of the alarm to update
 *                 example: "60f1b2b3b3b3b3b3b3b3b3b3"
 *               hour:
 *                 type: integer
 *                 minimum: 0
 *                 maximum: 23
 *                 description: Hour of the alarm (24-hour format)
 *                 example: 8
 *               minute:
 *                 type: integer
 *                 minimum: 0
 *                 maximum: 59
 *                 description: Minute of the alarm
 *                 example: 0
 *               daysOfWeek:
 *                 type: array
 *                 items:
 *                   type: integer
 *                   minimum: 0
 *                   maximum: 6
 *                 description: Days of the week (0=Sunday, 1=Monday, ..., 6=Saturday)
 *                 example: [1, 2, 3, 4, 5]
 *               enabled:
 *                 type: boolean
 *                 description: Whether the alarm is enabled
 *                 example: false
 *               label:
 *                 type: string
 *                 description: Label for the alarm
 *                 example: "Work alarm"
 *           example:
 *             _id: "60f1b2b3b3b3b3b3b3b3b3b3"
 *             hour: 8
 *             enabled: false
 *     responses:
 *       200:
 *         description: Alarm updated successfully
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Alarm'
 *       400:
 *         description: Invalid request data or alarm ID format
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       404:
 *         description: Alarm not found
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
export async function PATCH(request: NextRequest) {
  try {
    const body = await request.json();

    // Kiểm tra tính hợp lệ của dữ liệu yêu cầu
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

    // Kiểm tra định dạng ObjectId
    let objectId: ObjectId;
    try {
      objectId = new ObjectId(body._id);
    } catch {
      return NextResponse.json(
        { error: "Invalid alarm ID format" },
        { status: 400 }
      );
    }

    // Chuẩn bị trường dữ liệu để cập nhật
    const updateFields: Partial<Omit<Alarm, "_id">> = {
      hour: body.hour,
      minute: body.minute,
      daysOfWeek: body.daysOfWeek,
      enabled: body.enabled,
      label: body.label,
    };

    // Bỏ qua các trường không có giá trị
    const updateDoc: Record<string, unknown> = {};
    if (updateFields.hour !== undefined) updateDoc.hour = updateFields.hour;
    if (updateFields.minute !== undefined)
      updateDoc.minute = updateFields.minute;
    if (updateFields.daysOfWeek !== undefined)
      updateDoc.daysOfWeek = updateFields.daysOfWeek;
    if (updateFields.enabled !== undefined)
      updateDoc.enabled = updateFields.enabled;
    if (updateFields.label !== undefined) updateDoc.label = updateFields.label;

    // Không có trường hợp lệ nào để cập nhật
    if (Object.keys(updateDoc).length === 0) {
      return NextResponse.json(
        { error: "No valid fields to update" },
        { status: 400 }
      );
    }

    // Tiến hành cập nhật báo thức
    const result = await db.collection("alarms").findOneAndUpdate(
      { _id: objectId },
      { $set: updateDoc },
      { returnDocument: "after" } // trả về document sau khi cập nhật
    );

    // Xử lý kết quả trả về từ driver MongoDB (hỗ trợ cả phiên bản cũ và mới)
    const doc = result && result.value ? result.value : result;

    if (!doc) {
      // Không tìm thấy báo thức để cập nhật
      return NextResponse.json({ error: "Alarm not found" }, { status: 404 });
    }

    // Trả về báo thức đã cập nhật
    const updatedAlarm: Alarm = {
      hour: doc.hour,
      minute: doc.minute,
      enabled: doc.enabled,
      daysOfWeek: doc.daysOfWeek,
      label: doc.label,
      _id: doc._id.toString(),
    };

    return NextResponse.json(updatedAlarm);
  } catch (error) {
    console.error("Error updating alarm:", error);
    // Xảy ra lỗi trong quá trình xử lý
    return NextResponse.json(
      { error: "Failed to update alarm" },
      { status: 500 }
    );
  }
}
