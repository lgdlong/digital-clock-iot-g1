import { NextResponse } from "next/server";
import mqtt from "mqtt";

/**
 * @swagger
 * /api/esp32-time:
 *   get:
 *     summary: Get current time from ESP32
 *     description: Connect to MQTT broker and get the current time from ESP32 device
 *     tags:
 *       - ESP32
 *     responses:
 *       200:
 *         description: Current time from ESP32
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 time:
 *                   type: string
 *                   description: Current time in HH:MM:SS format
 *                   example: "14:30:25"
 *                 timestamp:
 *                   type: number
 *                   description: Unix timestamp when data was received
 *                 source:
 *                   type: string
 *                   description: Data source
 *                   example: "ESP32"
 *       500:
 *         description: Failed to connect to ESP32
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 error:
 *                   type: string
 *                   example: "Failed to connect to ESP32"
 */
export async function GET() {
  return await new Promise<Response>((resolve) => {
    try {
      console.log("[ESP32-TIME] Connecting to MQTT broker...");
      
      // Kết nối với MQTT broker (giống như ESP32)
      const client = mqtt.connect("mqtt://broker.emqx.io:1883", {
        clientId: `WebClient-${Math.random().toString(16).slice(2, 8)}`,
        connectTimeout: 10000,
        reconnectPeriod: 0, // Không reconnect
      });

      const timeoutId = setTimeout(() => {
        client.end();
        resolve(NextResponse.json(
          { error: "Timeout: ESP32 không phản hồi trong 10 giây" },
          { status: 500 }
        ));
      }, 10000);

      client.on("connect", () => {
        console.log("[ESP32-TIME] Connected to MQTT broker");
        
        // Subscribe để nhận thời gian từ ESP32
        client.subscribe("clock/time", (err) => {
          if (err) {
            console.error("[ESP32-TIME] Subscribe error:", err);
            clearTimeout(timeoutId);
            client.end();
            resolve(NextResponse.json(
              { error: "Failed to subscribe to ESP32 time topic" },
              { status: 500 }
            ));
          } else {
            console.log("[ESP32-TIME] Subscribed to clock/time topic");
            
            // Gửi lệnh reset để ESP32 publish thời gian ngay
            client.publish("clock/reset", "reset", (err) => {
              if (err) {
                console.error("[ESP32-TIME] Publish reset error:", err);
              } else {
                console.log("[ESP32-TIME] Reset command sent to ESP32");
              }
            });
          }
        });
      });

      client.on("message", (topic, message) => {
        if (topic === "clock/time") {
          const timeString = message.toString();
          console.log("[ESP32-TIME] Received time from ESP32:", timeString);
          
          clearTimeout(timeoutId);
          client.end();
          
          resolve(NextResponse.json({
            time: timeString,
            timestamp: Date.now(),
            source: "ESP32",
            status: "connected"
          }));
        }
      });

      client.on("error", (error) => {
        console.error("[ESP32-TIME] MQTT Error:", error);
        clearTimeout(timeoutId);
        client.end();
        resolve(NextResponse.json(
          { error: "MQTT connection error: " + error.message },
          { status: 500 }
        ));
      });

      client.on("close", () => {
        console.log("[ESP32-TIME] MQTT connection closed");
      });

    } catch (error) {
      console.error("[ESP32-TIME] Error:", error);
      resolve(NextResponse.json(
        { error: "Failed to connect to ESP32: " + (error as Error).message },
        { status: 500 }
      ));
    }
  });
}

/**
 * @swagger
 * /api/esp32-time:
 *   post:
 *     summary: Request time update from ESP32
 *     description: Send a reset command to ESP32 to trigger time publishing
 *     tags:
 *       - ESP32
 *     responses:
 *       200:
 *         description: Reset command sent successfully
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 message:
 *                   type: string
 *                   example: "Reset command sent to ESP32"
 *       500:
 *         description: Failed to send reset command
 */
export async function POST() {
  return await new Promise<Response>((resolve) => {
    try {
      console.log("[ESP32-TIME] Sending reset command to ESP32...");
      
      const client = mqtt.connect("mqtt://broker.emqx.io:1883", {
        clientId: `WebReset-${Math.random().toString(16).slice(2, 8)}`,
        connectTimeout: 5000,
        reconnectPeriod: 0,
      });

      const timeoutId = setTimeout(() => {
        client.end();
        resolve(NextResponse.json(
          { error: "Timeout connecting to MQTT broker" },
          { status: 500 }
        ));
      }, 5000);

      client.on("connect", () => {
        console.log("[ESP32-TIME] Connected to MQTT broker for reset");
        
        client.publish("clock/reset", "reset", (err) => {
          clearTimeout(timeoutId);
          client.end();
          
          if (err) {
            console.error("[ESP32-TIME] Failed to send reset:", err);
            resolve(NextResponse.json(
              { error: "Failed to send reset command" },
              { status: 500 }
            ));
          } else {
            console.log("[ESP32-TIME] Reset command sent successfully");
            resolve(NextResponse.json({
              message: "Reset command sent to ESP32",
              timestamp: Date.now()
            }));
          }
        });
      });

      client.on("error", (error) => {
        console.error("[ESP32-TIME] MQTT Error:", error);
        clearTimeout(timeoutId);
        client.end();
        resolve(NextResponse.json(
          { error: "MQTT error: " + error.message },
          { status: 500 }
        ));
      });

    } catch (error) {
      console.error("[ESP32-TIME] Error:", error);
      resolve(NextResponse.json(
        { error: "Failed to send reset command: " + (error as Error).message },
        { status: 500 }
      ));
    }
  });
} 