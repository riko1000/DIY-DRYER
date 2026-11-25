# Free hosted dashboard setup

The dashboard is hosted on Vercel. The Uno R4 WiFi makes an outbound HTTPS
request to the Vercel API every 10 seconds. The API stores only the latest
device status and a short-lived command in Supabase Postgres. The browser reads
that status from the API and queues commands there.

This uses HTTPS on port 443, not MQTT. There is no broker, no router
port-forwarding, and no device IP to enter. The dashboard requires a password
and is marked `noindex`.

## Safety note

The Uno handles HTTPS synchronously. A cloud request can pause the main loop
while the board connects and waits for a response, temporarily delaying sensor
checks and local heater control. The response timeout is configured to one
second, but connection setup may also take time; this is not a certified
maximum. Keep the dryer directly supervised and use an independent hardware
thermal cutoff. Do not rely on this dashboard or firmware as the only
over-temperature protection, and do not leave the dryer running unattended.

## 1. Create a Supabase Free project

1. Sign up at [supabase.com](https://supabase.com/) and create a project on the
   **Free** plan. Do not select Pro, paid compute, or paid add-ons.
2. Choose a database region near the school. Save the database password
   somewhere private; the firmware and Vercel API do not need that password.
3. In the project's SQL Editor, run the complete
   [`supabase/schema.sql`](../supabase/schema.sql) script. It creates one
   status row and two restricted database functions. The API uses the Supabase
   service-role key; browser code and the Arduino never receive that key.
4. From the project's **Settings > API Keys**, copy the **Project URL** and a
   **Secret key** (`sb_secret_...`). Do not use a Publishable key. The secret
   key is server-only and grants elevated access to the project.

Supabase currently lists a Free plan with a 500 MB database and 5 GB of
egress. Free projects may pause after a week of inactivity; a powered-on dryer
checks in regularly, but a long period with the board off may require manually
resuming the project. This design overwrites one status row instead of keeping
an ever-growing telemetry history.

## 2. Deploy to Vercel

1. Push the code to your GitHub repository and import it into Vercel. Keep the
   project root at the repository root.
2. Add these environment variables to the Vercel project:

   | Variable | Value |
   | --- | --- |
   | `SUPABASE_URL` | Supabase Project URL |
   | `SUPABASE_SECRET_KEY` | Supabase Secret key (`sb_secret_...`) |
   | `DEVICE_TOKEN` | A random secret of at least 32 bytes |
   | `DASHBOARD_PASSWORD` | Unique password, at least 16 characters |
   | `SESSION_SECRET` | A different random secret of at least 32 bytes |
   | `DRYER_DEVICE_ID` | `dryer-4338` |

   Generate a random secret in PowerShell with:

   ```powershell
   $bytes = New-Object byte[] 32
   $rng = [System.Security.Cryptography.RandomNumberGenerator]::Create()
   $rng.GetBytes($bytes)
   [Convert]::ToBase64String($bytes)
   $rng.Dispose()
   ```

   Run it separately for `DEVICE_TOKEN` and `SESSION_SECRET`. Keep both
   private. The device token must also be copied into the local firmware
   secrets file in the next step.

3. Deploy or redeploy. Vercel assigns a URL like
   `https://your-project.vercel.app`. The firmware needs the hostname only,
   without `https://` or a trailing slash.

## 3. Configure and upload the firmware

1. Copy `include\secrets.example.h` to `include\secrets.h`.
2. Set `WIFI_SSID` and `WIFI_PASSWORD` to a 2.4 GHz Wi-Fi network with internet
   access. Set `VERCEL_HOST` to the hostname from step 2 and `DEVICE_TOKEN` to
   the same secret used in Vercel. Keep `VERCEL_DEVICE_PATH` and
   `DRYER_DEVICE_ID` as shown unless you intentionally change the API route or
   device ID.
3. Build and upload the `uno_r4_wifi` PlatformIO environment, then open the
   serial monitor at 115200 baud. Expected messages include `Wi-Fi connected`
   and `Vercel API connected`.

`include\secrets.h` is ignored by Git. Never commit it, publish it, or put
secrets in the dashboard's static files.

## How it behaves

- Status and commands travel through HTTPS on port 443. The dryer checks in
  every 10 seconds, so updates and commands are not instantaneous.
- Commands are accepted only while the dryer was recently online. A command
  expires after 15 seconds and is not kept for later if the device is offline.
- The temperature controller, timer, and safety shutdowns stay on the Uno.
  Vercel or Supabase being unavailable does not take over local heater control.
- The DHT11 chamber sensor controls the heater, and its target is capped at
  48°C, within its 50°C rated range and with the controller's 2°C hysteresis.
  A separate thermistor shuts the dryer down at 80°C on the heatbed. Higher
  chamber targets and the corresponding material presets require a
  higher-range chamber sensor and matching firmware.
- A 100-pixel WS2815 strip connects to data pin D4. It displays dim RGB-mixed
  white while idle, a slow rainbow while drying, and red if the controller
  enters its error state. Brightness is deliberately limited.
- Power the WS2815 strip from a separate, regulated 12V supply sized for the
  exact strip. Do not power it from the Uno or connect 12V to an Arduino pin.
  Connect the strip supply ground to Arduino ground so the data signal has a
  common reference, and follow the strip manufacturer's wiring requirements.
- The dashboard is private behind its password, and its URL is not intended to
  be discoverable through Google Search.
