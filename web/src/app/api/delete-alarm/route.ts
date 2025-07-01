// app/api/delete-alarm/route.ts
import { NextRequest, NextResponse } from "next/server";
import { connectDB } from "@/config/db-config";
import { ObjectId } from "mongodb";

interface DeleteAlarmRequest {
  _id: string;
}

function validateDeleteAlarmData(data: unknown): data is DeleteAlarmRequest {
  if (typeof data !== "object" || data === null) return false;

  const obj = data as Record<string, unknown>;

  return typeof obj._id === "string" && obj._id.length > 0;
}

export async function DELETE(request: NextRequest) {
  try {
    const url = new URL(request.url);
    const queryId = url.searchParams.get("_id");

    let alarmId: string;

    // Try to get _id from query params first, then from body
    if (queryId) {
      alarmId = queryId;
    } else {
      const body = await request.json();

      // Validate request data
      if (!validateDeleteAlarmData(body)) {
        return NextResponse.json(
          { error: "Invalid request. Required: _id (string)" },
          { status: 400 }
        );
      }

      alarmId = body._id;
    }

    const db = await connectDB();

    // Validate ObjectId format
    let objectId: ObjectId;
    try {
      objectId = new ObjectId(alarmId);
    } catch {
      return NextResponse.json(
        { error: "Invalid alarm ID format" },
        { status: 400 }
      );
    }

    const result = await db.collection("alarms").deleteOne({ _id: objectId });

    if (result.deletedCount === 0) {
      return NextResponse.json({ error: "Alarm not found" }, { status: 404 });
    }

    return NextResponse.json({ success: true });
  } catch (error) {
    console.error("Error deleting alarm:", error);
    return NextResponse.json(
      { error: "Failed to delete alarm" },
      { status: 500 }
    );
  }
}
