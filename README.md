# Fire at the Farm Launch - Reefie v3L (No IMU)

### Flight Parameters

| Event            | Trigger Condition                                                | Sensor       |
|------------------|------------------------------------------------------------------|--------------|
| **Launch**       | Velocity > 10 m/s sustained for 250 ms                           | Barometer    |
| **Chute Deploy** | Light level > 40 sustained for 4000 ms                           | Light Sensor |
| **Disreef**      | Altitude AGL < 182.2 m (600 ft) sustained for 500 ms             | Barometer    |
| **Touchdown**    | Velocity < 2.5 m/s sustained for 1000 ms                         | Barometer    |

---

## Logged Data

Reefie logs the following parameters to onboard flash at 20 Hz:

| Parameter         | Logged |
|-------------------|--------|
| Altitude          | YES    |
| Velocity          | YES    |
| Pressure          | YES    |
| Acceleration      | YES    |
| Light Level       | YES    |
| Continuity Status | YES    |
| Battery Voltage   | YES    |
| Events Log        | YES    |
| Raw Altitude      | NO     |
| Temperature       | NO     |
| Gyroscope         | NO     |

---

## Full Build & Upload Instructions

Open Terminal in this project directory and run the following:

### 1. Clean previous build (optional but recommended)

```bash
pio run -t clean -e reefie_v3l
```

### 2. Build the filesystem image

```bash
pio run -t buildfs -e reefie_v3l
```

### 3. Upload the filesystem image

```bash
pio run -t uploadfs -e reefie_v3l
```

### 4. Build and upload the firmware

```bash
pio run -t upload -e reefie_v3l
```
