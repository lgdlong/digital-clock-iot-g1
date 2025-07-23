### How switch display mode betwen Clock + Temperature (line 402) and Weather Information (line 421)?

- In `displayClock()`, the LCD display mode is switched every 60 seconds using `lastLCDModeChange` and `now`:

```cpp
void displayClock()
{
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  // Switch display mode every 60 seconds
  if (now - lastLCDModeChange > 60000)
  {
    lcdDisplayMode = (lcdDisplayMode + 1) % 2;
    lastLCDModeChange = now;
  }
  ...
}
```

- `now - lastLCDModeChange > 60000` checks if 60 seconds have passed since the last mode change.
- If true, `lcdDisplayMode` is toggled between 0 and 1 (`(lcdDisplayMode + 1) % 2`).
- `lastLCDModeChange` is updated to the current time (`now`) after switching.
