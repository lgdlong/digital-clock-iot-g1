export interface Alarm {
  _id: string;
  hour: number; // 0–23
  minute: number; // 0–59
  enabled: boolean;
  daysOfWeek: number[]; // Danh sách các thứ trong tuần (0: Chủ Nhật, 1: Thứ 2, ..., 6: Thứ 7)
  label?: string;
}
