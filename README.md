# SolarKeeper

A project created by Ground Light, Lda. with the colaboration of Tiago Adrião.

This project was planned to give free information and awareness of the dangers of UV-A and UV-B exposure.

With this, we want to implement this information on every beach with informations about the realtime UV-B index and other informations like air temperature, tides, beach quality, etc.

With this we'll improve the UV-B index reading that are made nowadays via satellite.

This project have been prototiped with the support of a Arduino Wifi Rev.2 which reads and comunicates with a Raspberry 3B running Raspian and show the received information via a python-based GUI.

# SolarKeeper Setup Guide

This guide walks you through the steps to install required dependencies, configure your Raspberry Pi display, and set up the SolarKeeper GUI app as a systemd service.

---

## 🐍 Set Up Python Virtual Environment

```bash
sudo apt install python3-venv
python3 -m venv env --system-site-packages
source env/bin/activate
```

---

## 📦 Install Required Packages

```bash
cd ~
sudo apt-get update
sudo apt-get install -y git python3-pip
pip3 install --upgrade adafruit-python-shell click
```

---

## 💾 Clone and Run Adafruit PiTFT Installer

```bash
git clone https://github.com/adafruit/Raspberry-Pi-Installer-Scripts.git
cd Raspberry-Pi-Installer-Scripts
sudo -E env PATH=$PATH python3 adafruit-pitft.py --display=28r --rotation=0 --install-type=mirror
```

> 💡 This step configures the PiTFT 2.8" Resistive screen in mirror mode with 0° rotation.

---

## 🔌 Connect via SSH

From another machine, connect to your Raspberry Pi:

```bash
ssh solarkeeper@192.168.1.116
```

---

## ⚙️ Create a systemd Service for SolarKeeper GUI

Create and edit a systemd service file:

```bash
sudo nano /etc/systemd/system/solarkeeper.service
```

Paste the following configuration:

```ini
[Unit]
Description=SolarKeeper GUI App
After=graphical.target

[Service]
Type=simple
ExecStart=/home/solarkeeper/SolarKeeper/GUI_SurfBoard.py
Environment=DISPLAY=:0
Environment=XAUTHORITY=/home/solarkeeper/.Xauthority
User=solarkeeper
WorkingDirectory=/home/solarkeeper/SolarKeeper
Restart=on-failure

[Install]
WantedBy=graphical.target
```

---

## 🔄 Enable and Start the Service

```bash
sudo systemctl daemon-reexec
sudo systemctl enable solarkeeper.service
sudo systemctl restart solarkeeper.service
```

> ✅ Your SolarKeeper GUI should now start automatically on boot and restart on failure.
