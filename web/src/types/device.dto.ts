export interface Device {
  _id: string; // MongoDB ObjectId (string hóa)
  name: string; // Tên thiết bị do người dùng đặt
  userId: string; // ID người dùng sở hữu
  timezone: string; // Múi giờ (vd: 'Asia/Ho_Chi_Minh')
  status: "online" | "offline" | "unknown";
  ip?: string; // IP LAN mới nhất (tuỳ dùng)
  lastSeen?: Date; // Lần cuối đồng bộ server
  firmwareVersion?: string;
  createdAt: Date;
  updatedAt: Date;
}
