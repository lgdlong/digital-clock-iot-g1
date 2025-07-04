import { NextResponse } from "next/server";
import mqtt from "mqtt";

/**
 * @swagger
 * /api/esp32-status:
 *   get:
 *     summary: Check ESP32 connection status
 *     description: Test MQTT connection and get ESP32 device status
 *     tags:
 *       - ESP32
 *     responses:
 *       200:
 *         description: ESP32 status information
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 connected:
 *                   type: boolean
 *                   description: Whether ESP32 is connected
 *                 mqttBroker:
 *                   type: string
 *                   description: MQTT broker URL
 *                 latency:
 *                   type: number
 *                   description: Connection latency in ms
 *                 timestamp:
 *                   type: number
 *                   description: Test timestamp
 *                 topics:
 *                   type: object
 *                   properties:
 *                     timeTopic:
 *                       type: string
 *                     resetTopic:
 *                       type: string
 */
export async function GET() {
  const startTime = Date.now();
  
  return await new Promise<Response>((resolve) => {
    try {
      console.log("[ESP32-STATUS] Testing connection to ESP32...");
      
      const client = mqtt.connect("mqtt://broker.emqx.io:1883", {
        clientId: `StatusCheck-${Math.random().toString(16).slice(2, 8)}`,
        connectTimeout: 8000,
        reconnectPeriod: 0,
      });

      const timeoutId = setTimeout(() => {
        client.end();
        resolve(
          NextResponse.json({
            connected: false,
            mqttBroker: "broker.hivemq.com:1883",
            error: "Connection timeout",
            latency: Date.now() - startTime,
            timestamp: Date.now(),
            topics: {
              timeTopic: "clock/time",
              resetTopic: "clock/reset",
            },
          })
        );
      }, 8000);

      client.on("connect", () => {
        console.log("[ESP32-STATUS] Connected to MQTT broker");
        
        // Test subscribe
        client.subscribe("clock/time", (err) => {
          if (err) {
            console.error("[ESP32-STATUS] Subscribe error:", err);
            clearTimeout(timeoutId);
            client.end();
            resolve(
              NextResponse.json({
                connected: false,
                mqttBroker: "broker.hivemq.com:1883",
                error: "Subscribe failed: " + err.message,
                latency: Date.now() - startTime,
                timestamp: Date.now(),
                topics: {
                  timeTopic: "clock/time",
                  resetTopic: "clock/reset",
                },
              })
            );
          } else {
            console.log("[ESP32-STATUS] Successfully subscribed to clock/time");
            
            // Test publish
            client.publish("clock/reset", "status_check", (err) => {
              clearTimeout(timeoutId);
              client.end();
              
              if (err) {
                console.error("[ESP32-STATUS] Publish error:", err);
                resolve(
                  NextResponse.json({
                    connected: false,
                    mqttBroker: "broker.hivemq.com:1883",
                    error: "Publish failed: " + err.message,
                    latency: Date.now() - startTime,
                    timestamp: Date.now(),
                    topics: {
                      timeTopic: "clock/time",
                      resetTopic: "clock/reset",
                    },
                  })
                );
              } else {
                console.log("[ESP32-STATUS] MQTT broker connection successful");
                resolve(
                  NextResponse.json({
                    connected: true,
                    mqttBroker: "broker.hivemq.com:1883",
                    latency: Date.now() - startTime,
                    timestamp: Date.now(),
                    topics: {
                      timeTopic: "clock/time",
                      resetTopic: "clock/reset",
                    },
                    message: "MQTT broker connection test successful",
                  })
                );
              }
            });
          }
        });
      });

      client.on("error", (error) => {
        console.error("[ESP32-STATUS] MQTT Error:", error);
        clearTimeout(timeoutId);
        client.end();
        resolve(
          NextResponse.json({
            connected: false,
            mqttBroker: "broker.hivemq.com:1883",
            error: "MQTT error: " + error.message,
            latency: Date.now() - startTime,
            timestamp: Date.now(),
            topics: {
              timeTopic: "clock/time",
              resetTopic: "clock/reset",
            },
          })
        );
      });

    } catch (error) {
      console.error("[ESP32-STATUS] Error:", error);
      resolve(
        NextResponse.json({
          connected: false,
          mqttBroker: "broker.hivemq.com:1883",
          error: "Connection test failed: " + (error as Error).message,
          latency: Date.now() - startTime,
          timestamp: Date.now(),
          topics: {
            timeTopic: "clock/time",
            resetTopic: "clock/reset",
          },
        })
      );
    }
  });
} 