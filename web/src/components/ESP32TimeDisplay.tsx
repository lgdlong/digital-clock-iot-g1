"use client";
import { useState, useEffect } from "react";
import { FiClock, FiWifi, FiWifiOff, FiRefreshCw } from "react-icons/fi";

interface ESP32TimeDisplayProps {
  time: string;
  isConnected: boolean;
  onRefresh?: () => void;
}

interface ESP32Status {
  lastUpdate: number;
  source: string;
  latency: number;
}

export default function ESP32TimeDisplay({ 
  time, 
  isConnected, 
  onRefresh 
}: ESP32TimeDisplayProps) {
  const [status, setStatus] = useState<ESP32Status>({
    lastUpdate: 0,
    source: "Unknown",
    latency: 0
  });
  const [isRefreshing, setIsRefreshing] = useState(false);

  useEffect(() => {
    if (isConnected && time !== "00:00:00") {
      setStatus(prev => ({
        ...prev,
        lastUpdate: Date.now(),
        source: "ESP32"
      }));
    }
  }, [time, isConnected]);

  const handleRefresh = async () => {
    if (!onRefresh || isRefreshing) return;
    
    setIsRefreshing(true);
    const startTime = Date.now();
    
    try {
      await onRefresh();
      const latency = Date.now() - startTime;
      setStatus(prev => ({ ...prev, latency }));
    } catch (error) {
      console.error("Refresh failed:", error);
    } finally {
      setIsRefreshing(false);
    }
  };

  const getTimeSinceLastUpdate = () => {
    if (status.lastUpdate === 0) return "Chưa cập nhật";
    const seconds = Math.floor((Date.now() - status.lastUpdate) / 1000);
    if (seconds < 60) return `${seconds}s trước`;
    const minutes = Math.floor(seconds / 60);
    return `${minutes}m trước`;
  };

  return (
    <div className="card mb-4 border-primary">
      <div className="card-header bg-primary text-white d-flex justify-content-between align-items-center">
        <div className="d-flex align-items-center">
          <FiClock className="me-2" size={20} />
          <h5 className="mb-0">Thời gian ESP32</h5>
        </div>
        <div className="d-flex align-items-center">
          {isConnected ? (
            <span className="badge bg-success me-2">
              <FiWifi className="me-1" size={14} />
              Kết nối
            </span>
          ) : (
            <span className="badge bg-danger me-2">
              <FiWifiOff className="me-1" size={14} />
              Mất kết nối
            </span>
          )}
          <button
            className="btn btn-sm btn-outline-light"
            onClick={handleRefresh}
            disabled={isRefreshing || !isConnected}
            title="Đồng bộ thời gian"
          >
            <FiRefreshCw 
              className={isRefreshing ? "spin" : ""} 
              size={16} 
            />
          </button>
        </div>
      </div>
      
      <div className="card-body">
        <div className="row align-items-center">
          <div className="col-md-8">
            <div className="display-4 text-primary font-monospace text-center">
              {time}
            </div>
            <div className="text-center text-muted mt-2">
              <small>
                {isConnected ? "Đồng bộ trực tiếp từ ESP32" : "Không có kết nối"}
              </small>
            </div>
          </div>
          
          <div className="col-md-4">
            <div className="text-center">
              <h6 className="text-muted mb-3">Thông tin kết nối</h6>
              
              <div className="mb-2">
                <small className="text-muted d-block">Nguồn dữ liệu:</small>
                <span className="badge bg-info">{status.source}</span>
              </div>
              
              <div className="mb-2">
                <small className="text-muted d-block">Cập nhật lần cuối:</small>
                <span className="text-primary">{getTimeSinceLastUpdate()}</span>
              </div>
              
              {status.latency > 0 && (
                <div className="mb-2">
                  <small className="text-muted d-block">Độ trễ:</small>
                  <span className="text-success">{status.latency}ms</span>
                </div>
              )}
              
              <div className="mt-3">
                <small className="text-muted d-block">Trạng thái MQTT:</small>
                <div className={`badge ${isConnected ? 'bg-success' : 'bg-secondary'}`}>
                  {isConnected ? 'Hoạt động' : 'Ngắt kết nối'}
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
      
      {!isConnected && (
        <div className="card-footer bg-warning-subtle">
          <div className="d-flex align-items-center text-warning-emphasis">
            <FiWifiOff className="me-2" />
            <small>
              <strong>Lưu ý:</strong> Không thể kết nối tới ESP32. 
              Vui lòng kiểm tra thiết bị và kết nối internet.
            </small>
          </div>
        </div>
      )}
    </div>
  );
}

// Custom CSS để animation loading
const styles = `
  .spin {
    animation: spin 1s linear infinite;
  }
  
  @keyframes spin {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
  }
`;

// Thêm styles vào document
if (typeof document !== 'undefined') {
  const styleSheet = document.createElement("style");
  styleSheet.innerText = styles;
  document.head.appendChild(styleSheet);
} 