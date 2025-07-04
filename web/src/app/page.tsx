"use client";
import ClockDisplay from "@/components/ClockDisplay";
import DeviceStatus from "@/components/DeviceStatus";
import NextAlarm from "@/components/NextAlarm";
import ESP32TimeDisplay from "@/components/ESP32TimeDisplay";
import Link from "next/link";
import { useState, useEffect } from "react";
import mqtt from "mqtt";

export default function Home() {
  const [currentTime, setCurrentTime] = useState("00:00:00");
  const [isOnline, setIsOnline] = useState(false);
  const [mqttClient, setMqttClient] = useState<mqtt.MqttClient | null>(null);
  const [isMounted, setIsMounted] = useState(false);
  const [connectionError, setConnectionError] = useState<string>("");
  const [lastMqttUpdate, setLastMqttUpdate] = useState<number>(0);
  const nextAlarm = "06:30";

  useEffect(() => {
    setIsMounted(true);
  }, []);

  useEffect(() => {
    if (!isMounted) return;

    let reconnectTimer: NodeJS.Timeout;
    let client: mqtt.MqttClient;

    const connectMQTT = () => {
      console.log('🔄 Đang kết nối tới ESP32 qua MQTT...');
      setConnectionError("");
      
      // Kết nối WebSocket cho browser
      // Thử các MQTT broker khác nhau
      const mqttUrls = [
        'wss://broker.hivemq.com:8884/mqtt',  // HiveMQ SSL
        'wss://broker.emqx.io:8084/mqtt',     // EMQX public
        'wss://test.mosquitto.org:8081',      // Mosquitto test
        'wss://broker.hivemq.com:8000/mqtt'   // HiveMQ WebSocket (fallback)
      ];
      
      let currentUrlIndex = 0;
      
      const tryConnect = () => {
        if (currentUrlIndex >= mqttUrls.length) {
          console.error('❌ Tất cả MQTT brokers đều không kết nối được');
          setIsOnline(false);
          setConnectionError("Không thể kết nối tới MQTT broker nào");
          return;
        }
        
        const mqttUrl = mqttUrls[currentUrlIndex];
        console.log(`🔄 Thử kết nối MQTT: ${mqttUrl}`);
        
        client = mqtt.connect(mqttUrl, {
          clientId: `WebApp-${Math.random().toString(16).slice(2, 8)}`,
          connectTimeout: 15000, // Tăng timeout lên 15s
          keepalive: 60,
          clean: true,
          reconnectPeriod: 0,
        });
        
        client.on('connect', () => {
          console.log(`✅ Đã kết nối MQTT broker: ${mqttUrl}`);
          setIsOnline(true);
          setConnectionError("");
          
          // Subscribe để nhận thời gian từ ESP32
          client.subscribe('clock/time', (err) => {
            if (err) {
              console.error('❌ Lỗi subscribe:', err);
              setConnectionError("Không thể subscribe topic thời gian");
            } else {
              console.log('📡 Đã subscribe topic clock/time');
              
              // Gửi lệnh để ESP32 publish thời gian ngay
              client.publish('clock/reset', 'reset');
              console.log('📤 Đã gửi lệnh reset tới ESP32');
            }
          });
        });

        client.on('error', (error: Error) => {
          console.error(`❌ Lỗi MQTT với ${mqttUrl}:`, error);
          currentUrlIndex++;
          if (client) {
            client.removeAllListeners();
            client.end();
          }
          setTimeout(tryConnect, 2000); // Thử broker tiếp theo sau 2s
        });

        client.on('close', () => {
          console.log(`🔌 MQTT đã ngắt kết nối: ${mqttUrl}`);
          setIsOnline(false);
          
          // Auto reconnect với broker hiện tại
          setTimeout(() => {
            if (currentUrlIndex < mqttUrls.length) {
              tryConnect();
            }
          }, 3000);
        });

        client.on('offline', () => {
          console.log(`📡 MQTT offline: ${mqttUrl}`);
          setIsOnline(false);
          setConnectionError("Mất kết nối internet");
        });
      };
      
      tryConnect();
      
      client.on('connect', () => {
        console.log('✅ Đã kết nối MQTT broker');
        setIsOnline(true);
        setConnectionError("");
        
        // Subscribe để nhận thời gian từ ESP32
        client.subscribe('clock/time', (err) => {
          if (err) {
            console.error('❌ Lỗi subscribe:', err);
            setConnectionError("Không thể subscribe topic thời gian");
          } else {
            console.log('📡 Đã subscribe topic clock/time');
            
            // Gửi lệnh để ESP32 publish thời gian ngay
            client.publish('clock/reset', 'reset');
            console.log('📤 Đã gửi lệnh reset tới ESP32');
          }
        });
      });

      client.on('message', (topic: string, message: any) => {
        if (topic === 'clock/time') {
          const timeFromESP32 = message.toString();
          console.log('⏰ Nhận từ ESP32:', timeFromESP32);
          // Kiểm tra định dạng HH:MM:SS
          const timeRegex = /^\d{2}:\d{2}:\d{2}$/;
          if (timeRegex.test(timeFromESP32)) {
            setCurrentTime(timeFromESP32);
            setConnectionError("");
          } else {
            setConnectionError(`Dữ liệu giờ từ ESP32 không hợp lệ: ${timeFromESP32}`);
            // Không cập nhật currentTime nếu sai định dạng
          }
        }
      });

      client.on('error', (error: Error) => {
        console.error('❌ Lỗi MQTT:', error);
        setIsOnline(false);
        setConnectionError(`Lỗi kết nối: ${error.message}`);
        
        // Auto reconnect sau 5 giây
        reconnectTimer = setTimeout(() => {
          if (client) client.end();
          connectMQTT();
        }, 5000);
      });

      client.on('close', () => {
        console.log('🔌 MQTT đã ngắt kết nối');
        setIsOnline(false);
        
        // Auto reconnect sau 3 giây
        reconnectTimer = setTimeout(() => {
          connectMQTT();
        }, 3000);
      });

      client.on('offline', () => {
        console.log('📡 MQTT offline');
        setIsOnline(false);
        setConnectionError("Mất kết nối internet");
      });

      setMqttClient(client);
    };

    connectMQTT();

    return () => {
      if (reconnectTimer) clearTimeout(reconnectTimer);
      if (client) {
        client.removeAllListeners();
        client.end(true);
      }
    };
  }, [isMounted]);

  // Local time increment
  useEffect(() => {
    if (!isMounted || currentTime === "00:00:00") return;

    const interval = setInterval(() => {
      setCurrentTime((prevTime: string) => {
        const [hours, minutes, seconds] = prevTime.split(':').map(Number);
        let newSeconds = seconds + 1;
        let newMinutes = minutes;
        let newHours = hours;

        if (newSeconds >= 60) {
          newSeconds = 0;
          newMinutes += 1;
        }
        if (newMinutes >= 60) {
          newMinutes = 0;
          newHours += 1;
        }
        if (newHours >= 24) {
          newHours = 0;
        }

        return `${newHours.toString().padStart(2, '0')}:${newMinutes.toString().padStart(2, '0')}:${newSeconds.toString().padStart(2, '0')}`;
      });
    }, 1000);

    return () => clearInterval(interval);
  }, [isMounted, currentTime]);

  const handleResetTime = () => {
    if (mqttClient && mqttClient.connected) {
      mqttClient.publish('clock/reset', 'reset');
      console.log('🔄 Đã gửi lệnh reset tới ESP32');
    } else {
      console.error('❌ MQTT client chưa kết nối');
      setConnectionError("Không thể gửi lệnh - MQTT chưa kết nối");
    }
  };

  // Function để refresh thời gian cho ESP32TimeDisplay component
  const handleRefreshTime = async () => {
    return new Promise<void>((resolve, reject) => {
      if (!mqttClient || !mqttClient.connected) {
        reject(new Error("MQTT chưa kết nối"));
        return;
      }

      const timeout = setTimeout(() => {
        reject(new Error("Timeout - ESP32 không phản hồi"));
      }, 10000);

      const messageHandler = (topic: string) => {
        if (topic === 'clock/time') {
          clearTimeout(timeout);
          mqttClient.off('message', messageHandler);
          resolve();
        }
      };

      mqttClient.on('message', messageHandler);
      mqttClient.publish('clock/reset', 'reset');
    });
  };

  return (
    <div className="container py-5">
      {/* Component hiển thị thời gian ESP32 mới */}
      <ESP32TimeDisplay 
        time={isMounted ? currentTime : "00:00:00"}
        isConnected={isMounted ? isOnline : false}
        onRefresh={handleRefreshTime}
      />
      
      {/* Hiển thị lỗi kết nối nếu có */}
      {connectionError && (
        <div className="alert alert-warning alert-dismissible fade show" role="alert">
          <i className="bi bi-exclamation-triangle-fill me-2"></i>
          <strong>Cảnh báo kết nối:</strong> {connectionError}
          <button 
            type="button" 
            className="btn-close" 
            onClick={() => setConnectionError("")}
            aria-label="Close"
          ></button>
        </div>
      )}

      {/* Component clock display truyền thống (backup) */}
      <ClockDisplay time={isMounted ? currentTime : "00:00:00"} />
      
      <DeviceStatus online={isMounted ? isOnline : false} />
      <NextAlarm alarmTime={nextAlarm} />
      
      <div className="my-3 text-center">
        <button 
          onClick={handleResetTime} 
          className="btn btn-warning me-3"
          disabled={!isMounted || !isOnline}
        >
          🔄 Đồng bộ thời gian
        </button>
        <Link href="/alarm" className="btn btn-primary">
          + Thêm báo thức
        </Link>
      </div>

      {/* Thông tin debug */}
      {process.env.NODE_ENV === 'development' && (
        <div className="mt-4">
          <div className="card bg-light">
            <div className="card-body">
              <h6 className="card-title">🔧 Debug Info</h6>
              <small className="text-muted">
                <div>MQTT Connected: {isOnline ? '✅' : '❌'}</div>
                <div>Current Time: {currentTime}</div>
                <div>Last Update: {lastMqttUpdate ? new Date(lastMqttUpdate).toLocaleTimeString() : 'Chưa có'}</div>
                <div>Connection Error: {connectionError || 'Không có'}</div>
              </small>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
