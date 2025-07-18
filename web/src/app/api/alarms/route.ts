// app/api/alarms/route.ts
import { NextResponse } from "next/server";
import { connectDB } from "@/config/db-config";
import type { Alarm } from "@/types/alarm.dto";

/**
 * @swagger
 * /api/alarms:
 *   get:
 *     summary: Get all alarms
 *     description: Retrieve all alarms from the database, sorted by hour and minute
 *     tags:
 *       - Alarms
 *     responses:
 *       200:
 *         description: List of all alarms
 *         content:
 *           application/json:
 *             schema:
 *               type: array
 *               items:
 *                 $ref: '#/components/schemas/Alarm'
 *       500:
 *         description: Internal server error
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 */
export async function GET() {
  try {
    const db = await connectDB();
    const alarms = await db
      .collection<Alarm>("alarms")
      .find({})
      .sort({ hour: 1, minute: 1 }) // Sắp xếp theo giờ, phút tăng dần
      .toArray();


    // Chuyển _id ObjectId về string (nếu dùng type Alarm với _id: string)
    const alarmsTransformed = alarms.map((alarm) => ({
      ...alarm,
      _id: alarm._id.toString(),
    }));

    // Log all alarms after transform
    console.log(
      "[GET /api/alarms] All alarms sent to client:",
      alarmsTransformed
    );

    return NextResponse.json(alarmsTransformed);
  } catch (error) {
    console.error("Error fetching alarms:", error);
    return NextResponse.json(
      { error: "Failed to fetch alarms" },
      { status: 500 }
    );
  }
}
