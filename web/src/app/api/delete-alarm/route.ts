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

/**
 * @swagger
 * /api/delete-alarm:
 *   delete:
 *     summary: Delete an alarm
 *     description: Delete an existing alarm by its ID
 *     tags:
 *       - Alarms
 *     parameters:
 *       - in: query
 *         name: _id
 *         required: false
 *         schema:
 *           type: string
 *         description: Alarm ID to delete (can also be provided in request body)
 *         example: "60f1b2b3b3b3b3b3b3b3b3b3"
 *     requestBody:
 *       required: false
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               _id:
 *                 type: string
 *                 description: Alarm ID to delete
 *                 example: "60f1b2b3b3b3b3b3b3b3b3b3"
 *     responses:
 *       200:
 *         description: Alarm deleted successfully
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Success'
 *       400:
 *         description: Invalid request or alarm ID format
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
