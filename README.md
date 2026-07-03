# Smart EV Charging Network Simulator

A command-line interface (CLI) smart EV charging station simulator built in Modern C++. It models real-time station management, physical power bottlenecks, and dynamic grid pricing tariffs with strict memory safety.

## ✨ Key Features
* **Dual-Tier Charging:** Models AC Slow Charging (7kW) and DC Fast Charging (50kW) points.
* **Physics-Based Constraints:** Computes true energy transfer using $\min(\text{chargerPower}, \text{vehicleMaxIntake})$.
* **Smart Grid Dynamic Pricing:** Simulates a time-of-use tariff with a **1.5x premium surcharge** automatically applied during peak hours (18:00 to 22:00).
* **Automated Lifecycles:** Vehicles auto-evict and clear from RAM upon reaching $100\%$ State of Charge (SOC) or when manually undocked with a full receipt.

## 🛠️ Tech Stack & Architecture
* **Language:** Modern C++ (C++14 or newer)
* **OOP Concepts:** Inheritance, Encapsulation, and Polymorphism.
* **Memory Management:** RAII design using `std::unique_ptr` and move semantics (`std::move`) to seamlessly transfer vehicle ownership directly into charging slots without memory leaks.

## 🚀 Getting Started

### Compilation
Compile using any standard C++ compiler:
```bash
g++ -std=c++14 main.cpp -o ev_simulator
Running the App
Bash
./ev_simulator
📊 Management Console Options
View Charging Stations Status: Check live occupancy, vehicle IDs, and current SOC% metrics.

Create & Dock a Vehicle: Provision a new vehicle profile and hand over its memory ownership to a slot.

Advance Simulation Time: Move the clock forward to process power delivery and trigger billing calculations.

Undock/Unplug a Vehicle: Manually interrupt a session to generate an itemized invoice in Rs. based on net energy consumed.

Exit Simulation: Cleanly terminate all active objects.