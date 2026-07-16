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