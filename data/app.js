const chamberTemp = document.getElementById("chamberTemp");
const heatbedTemp = document.getElementById("heatbedTemp");
const humidity = document.getElementById("humidity");

const statusBadge = document.getElementById("statusBadge");

const progressBar = document.getElementById("progressBar");
const remainingTime = document.getElementById("remainingTime");

const targetSlider = document.getElementById("targetTemp");
const targetValue = document.getElementById("targetValue");

const startButton = document.getElementById("startButton");
const stopButton = document.getElementById("stopButton");

const timeSlider = document.getElementById("timeSlider");
const timeValue = document.getElementById("timeValue");

timeSlider.addEventListener("input", () =>
{
    timeValue.textContent = timeSlider.value;
});

targetSlider.addEventListener("input", () =>
{
    targetValue.textContent = targetSlider.value;
});

startButton.addEventListener("click", async () =>
{
    await fetch("/api/start",
    {
        method: "POST"
    });
});

stopButton.addEventListener("click", async () =>
{
    await fetch("/api/stop",
    {
        method: "POST"
    });
});

const presets = {
    PLA:  { temp: 45, hours: 6 },
    PETG: { temp: 65, hours: 6 },
    ABS:  { temp: 70, hours: 4 },
    ASA:  { temp: 75, hours: 4 },
    TPU:  { temp: 50, hours: 8 },
    PA:   { temp: 70, hours: 12 },
    PC:   { temp: 75, hours: 8 }
};


async function sendSettings()
{
    await fetch("/api/settings", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({
            temperature: Number(targetSlider.value),
            hours: Number(timeSlider.value)
        })
    });
}



document.querySelectorAll(".materials button").forEach(button =>
{
    
    button.addEventListener("click", async () =>    {
        
        const name = button.textContent.trim();

    if (name === "Custom")
    return;

    const preset = presets[name];

        targetSlider.value = preset.temp;
        targetValue.textContent = preset.temp;

        timeSlider.value = preset.hours;
        timeValue.textContent = preset.hours;

        await sendSettings();
    });
});

async function updateStatus()
{
    try
    {
        const response = await fetch("/api/status");

        const data = await response.json();

        chamberTemp.textContent =
            data.chamber.toFixed(1) + " °C";

        heatbedTemp.textContent =
            data.heatbed.toFixed(1) + " °C";

        humidity.textContent =
            data.humidity.toFixed(0) + " %";

        progressBar.value = data.progress;

        remainingTime.textContent =
            data.remaining;

        statusBadge.textContent =
            "● " + data.state.toUpperCase();

        statusBadge.className =
            "status " + data.state.toLowerCase();
    }
    catch(error)
    {
        statusBadge.textContent = "● OFFLINE";

        statusBadge.className = "status error";
    }
}

updateStatus();

setInterval(updateStatus, 1000);

