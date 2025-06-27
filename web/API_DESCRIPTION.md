[] GET /api/get-device-time
res:
    {
        "datetime": "2024-06-28T13:42:21", // ISO 8601
        "date": "2024-06-28",
        "time": "13:42:21"
    }

[] GET /api/alarms
res:
    [
        { "id": 1, "datetime": "2024-06-29T06:30:00", "enabled": true },
        { "id": 2, "datetime": "2024-06-29T21:15:00", "enabled": false }
    ]
// Nếu muốn mở rộng báo thức lặp lại
// { "id": 3, "datetime": "2024-06-30T07:00:00", "enabled": true, "repeat": "Mon,Wed,Fri" }

[] POST /api/set-alarm
body:
    {
        "datetime": "2024-06-30T07:00:00",
        "enabled": true
    }
res:
    {
        "success": true,
        "alarm": { "id": 3, "datetime": "2024-06-30T07:00:00", "enabled": true }
    }


[] DELETE /api/delete-alarm?id=1
res:
    {
        "success": true
    }
