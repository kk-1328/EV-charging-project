#include <iostream>
#include <vector>
#include <string>
#include <memory>

// ==========================================
// 1. VEHICLE MODULE (Inheritance & Polymorphism)
// ==========================================
class Vehicle {
protected:
    std::string vehicleID;
    double batteryCapacity; // in kWh
    double currentSOC;      // State of Charge percentage (0.0 to 100.0)
    double sessionEnergyConsumed = 0.0;

public:
    Vehicle(std::string id, double capacity, double soc) 
        : vehicleID(id), batteryCapacity(capacity), currentSOC(soc) {}

    virtual ~Vehicle() {}

    // Different vehicles have different max intake speeds
    virtual double getMaxChargingSpeed() const = 0;
    
    std::string getID() const { return vehicleID; }
    double getCapacity() const { return batteryCapacity; }
    double getSOC() const { return currentSOC; }
    
   void updateSOC(double energyAdded) {
    double percentageGained = (energyAdded / batteryCapacity) * 100.0;
    currentSOC += percentageGained;
    if (currentSOC > 100.0) currentSOC = 100.0;
    
    sessionEnergyConsumed += energyAdded; // Add to session total
}

double getSessionEnergy() const { return sessionEnergyConsumed; }
void resetSessionEnergy() { sessionEnergyConsumed = 0.0; }
};

// Derived Class 1: Electric Bike
class ElectricBike : public Vehicle {
public:
    ElectricBike(std::string id, double soc) : Vehicle(id, 4.0, soc) {} // 4 kWh battery

    double getMaxChargingSpeed() const override {
        return 5.0; // Max 5 kW intake speed
    }
};

// Derived Class 2: Electric Car
class ElectricCar : public Vehicle {
public:
    ElectricCar(std::string id, double soc) : Vehicle(id, 60.0, soc) {} // 60 kWh battery

    double getMaxChargingSpeed() const override {
        return 50.0; // Max 50 kW intake speed
    }
};

// ==========================================
// 2. CHARGING POINT MODULE (Encapsulation)
// ==========================================
enum class ChargerType { AC_SLOW, DC_FAST };

class ChargingPoint {
private:
    int id;
    ChargerType type;
    double continuousPowerOutput;
    std::unique_ptr<Vehicle> occupiedBy; // Now owns the vehicle!

public:
    ChargingPoint(int p_id, ChargerType t) : id(p_id), type(t), occupiedBy(nullptr) {
        continuousPowerOutput = (type == ChargerType::AC_SLOW) ? 7.0 : 50.0;
    }

    bool isAvailable() const { return occupiedBy == nullptr; }
    
    // Use std::move to transfer ownership into the slot
    void plugVehicle(std::unique_ptr<Vehicle> v) { occupiedBy = std::move(v); }
    
    // Release ownership completely (destroys the object or lets us clear it)
    void unplugVehicle() { occupiedBy.reset(); }
    
    Vehicle* getVehicle() const { return occupiedBy.get(); }
    double getPowerOutput() const { return continuousPowerOutput; }
    std::string getTypeString() const {
        return (type == ChargerType::AC_SLOW) ? "AC Slow (7kW)" : "DC Fast (50kW)";
    }
};

// ==========================================
// 3. CHARGING STATION MODULE
// ==========================================
class ChargingStation {
private:
    int stationID;
    std::string name;
    std::vector<ChargingPoint> slots;

public:
    ChargingStation(int id, std::string s_name, int numAC, int numDC) 
        : stationID(id), name(s_name) {
        
        int currentSlotID = 1;
        for (int i = 0; i < numAC; ++i) {
            slots.push_back(ChargingPoint(currentSlotID++, ChargerType::AC_SLOW));
        }
        for (int i = 0; i < numDC; ++i) {
            slots.push_back(ChargingPoint(currentSlotID++, ChargerType::DC_FAST));
        }
    }

    std::string getName() const { return name; }
    int getID() const { return stationID; }
    const std::vector<ChargingPoint>& getSlots() const { return slots; }

    void displayStatus() const {
        std::cout << "\n--- Station: " << name << " (ID: " << stationID << ") ---\n";
        for (const auto& slot : slots) {
            std::cout << "Slot " << " | " << slot.getTypeString() << " | ";
            if (slot.isAvailable()) {
                std::cout << "AVAILABLE\n";
            } else {
                Vehicle* v = slot.getVehicle();
                std::cout << "OCCUPIED by [" << v->getID() << "] | SOC: " 
                          << v->getSOC() << "%\n";
            }
        }
    }
 
    // Helper function to check if the station has ANY free space
bool hasAvailableSlots() const {
    for (const auto& slot : slots) {
        if (slot.isAvailable()) return true;
    }
    return false;
}

bool dockVehicle(int slotIndex, std::unique_ptr<Vehicle> v) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(slots.size())) {
        std::cout << "[ERROR] Invalid slot number.\n";
        return false;
    }
    if (!slots[slotIndex].isAvailable()) {
        std::cout << "[ERROR] Slot already occupied.\n";
        return false;
    }
    
    std::cout << "[SUCCESS] Vehicle [" << v->getID() << "] docked.\n";
    slots[slotIndex].plugVehicle(std::move(v)); // Ownership moves here
    return true;
}
// Manually unplug a vehicle and process the final bill
bool undockVehicle(int slotIndex, double pricingRatePerKWh) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(slots.size())) {
        std::cout << "[ERROR] Invalid slot number!\n";
        return false;
    }

    if (slots[slotIndex].isAvailable()) {
        std::cout << "[ERROR] Slot " << slotIndex + 1 << " is already empty!\n";
        return false;
    }

    Vehicle* v = slots[slotIndex].getVehicle();
    
    double totalEnergyUsed = v->getSessionEnergy();
    double finalBill = totalEnergyUsed * pricingRatePerKWh;

    std::cout << "\n=========================================\n";
    std::cout << "         CHARGING SESSION RECEIPT        \n";
    std::cout << "=========================================\n";
    std::cout << " Station        : " << name << "\n";
    std::cout << " Vehicle ID     : " << v->getID() << "\n";
    std::cout << " Slot Number    : " << slotIndex + 1 << " (" << slots[slotIndex].getTypeString() << ")\n";
    std::cout << " Final Battery  : " << v->getSOC() << "%\n";
    std::cout << "-----------------------------------------\n";
    std::cout << " Total Energy   : " << totalEnergyUsed << " kWh\n";
    std::cout << " Tariff Rate    : Rs. " << pricingRatePerKWh << " / kWh\n";
    std::cout << " TOTAL AMOUNT   : Rs. " << finalBill << "\n";
    std::cout << "=========================================\n";
    std::cout << "[INFO] Slot " << slotIndex + 1 << " is now FREE.\n";

    v->resetSessionEnergy();
    slots[slotIndex].unplugVehicle();
    
    return true;
}
void advanceTime(double hours, double pricingRatePerKWh) {
    for (size_t i = 0; i < slots.size(); ++i) {
        if (slots[i].isAvailable()) continue;

        Vehicle* v = slots[i].getVehicle();
        
        double chargerPower = slots[i].getPowerOutput();
        double vehicleMax = v->getMaxChargingSpeed();
        double actualPowerRate = (chargerPower < vehicleMax) ? chargerPower : vehicleMax;

        double remainingCapacityNeeded = v->getCapacity() * (1.0 - (v->getSOC() / 100.0));
        
        double potentialEnergySupplied = actualPowerRate * hours;
        
        double energyConsumed = (potentialEnergySupplied > remainingCapacityNeeded) 
                                ? remainingCapacityNeeded 
                                : potentialEnergySupplied;
        
        v->updateSOC(energyConsumed);
        double cost = energyConsumed * pricingRatePerKWh;

        if (v->getSOC() >= 100.0) {
            std::cout << "\n [INFO] [NOTIFICATION] Vehicle [" << v->getID() 
                      << "] is fully charged at " << name << "!\n";
            std::cout << "   Energy Consumed: " << energyConsumed << " kWh | Total Cost: Rs" << cost << "\n";
            
            slots[i].unplugVehicle(); 
        } else {
            std::cout << " > [" << v->getID() << "] charged +" << energyConsumed 
                      << " kWh (Current SOC: " << v->getSOC() << "%)\n";
        }
    }
}
};
int main() {
    // 1. Initialize our mock network stations
    std::vector<ChargingStation> network;
    network.push_back(ChargingStation(101, "Malviya Nagar - Jaipur", 2, 1)); 
    network.push_back(ChargingStation(102, "Civil Lines - Jaipur", 1, 2));  

    // 2. Track actively managed vehicle objects in our system memory
    std::vector<std::unique_ptr<Vehicle>> activeVehicles;
    
    // Create initial mock vehicles
    activeVehicles.push_back(std::make_unique<ElectricCar>("CAR-01", 20.0));   
    activeVehicles.push_back(std::make_unique<ElectricBike>("BIKE-99", 45.0)); 

    int choice = 0;
    double basePricePerKWh = 9.50; // Base tariff: Rs. 9.50 per unit
    int currentHour = 14;          // Simulation starts at 14:00 (2:00 PM)

    std::cout << "========================================================\n";
    std::cout << "   SMART GRID EV CHARGING NETWORK ENGINE ACTIVATED \n";
    std::cout << "========================================================\n";

    while (choice != 5) {
        // Calculate dynamic tariff rate based on time of day
        double activeTariffRate = basePricePerKWh;
        std::string pricingStatus = "STANDARD RATE";
        
        // Peak hours rule: 6 PM (18) to 10 PM (22)
        if (currentHour >= 18 && currentHour <= 22) {
            activeTariffRate = basePricePerKWh * 1.5;
            pricingStatus = "PEAK HIGH-DEMAND RATE (1.5x SURCHARGE)";
        }

        // Format hour display neatly (e.g., 14:00)
        std::cout << "\n========================================================\n";
        std::cout << " [SYSTEM TIME] " << (currentHour < 10 ? "0" : "") << currentHour << ":00 | " 
                  << "Current Tariff: Rs. " << activeTariffRate << " / kWh (" << pricingStatus << ")\n";
        std::cout << "========================================================\n";

        std::cout << "1. View Charging Stations Status\n";
        std::cout << "2. Dock/Plug a Vehicle into a Slot\n";
        std::cout << "3. Advance Simulation Time (Step Progression)\n";
        std::cout << "4. Undock/Unplug a Vehicle (Generate Bill)\n";
        std::cout << "5. Exit Simulation\n";
        std::cout << "Enter selection: ";
        std::cin >> choice;

        if (choice == 1) {
            for (const auto& station : network) {
                station.displayStatus();
            }
        } 
       else if (choice == 2) {
    std::cout << "\nSelect a Target Station:\n";
    for (size_t i = 0; i < network.size(); ++i) {
        std::cout << i + 1 << ". " << network[i].getName() << "\n";
    }
    int sIndex; std::cin >> sIndex; sIndex--;

    if (sIndex >= 0 && sIndex < static_cast<int>(network.size())) {
        if (!network[sIndex].hasAvailableSlots()) {
            std::cout << "[ERROR] No slots available at this station!\n";
        } else {
            // 1. Create the vehicle directly at the gate
            std::cout << "\nVehicle Type? (1 for Car, 2 for Bike): ";
            int type; std::cin >> type;
            std::cout << "Enter Registration ID: ";
            std::string id; std::cin >> id;
            std::cout << "Enter current Battery %: ";
            double soc; std::cin >> soc;

            std::unique_ptr<Vehicle> newVehicle;
            if (type == 1) newVehicle = std::make_unique<ElectricCar>(id, soc);
            else newVehicle = std::make_unique<ElectricBike>(id, soc);

            // 2. Choose the slot
            network[sIndex].displayStatus();
            std::cout << "Enter an available slot number to dock: ";
            int slotNum; std::cin >> slotNum;

            // 3. Move it directly into the station. main() completely forgets about it now!
            network[sIndex].dockVehicle(slotNum - 1, std::move(newVehicle));
        }
    }
}
       
        else if (choice == 3) {
            std::cout << "\nEnter time duration to advance forward (in minutes): ";
            double minutes; std::cin >> minutes;
            double hours = minutes / 60.0;

            std::cout << "\n--- Advancing Network Time by " << minutes << " Minutes ---\n";
            
            // Execute time step across all charging units using the active dynamic rate
            for (auto& station : network) {
                station.advanceTime(hours, activeTariffRate);
            }

            // Update the global clock hour
            int hoursToAdvance = static_cast<int>(minutes) / 60;
            currentHour = (currentHour + hoursToAdvance) % 24; // Keep it within 00:00 to 23:00
        } 
        else if (choice == 4) {
            std::cout << "\nSelect the Station to unplug from:\n";
            for (size_t i = 0; i < network.size(); ++i) {
                std::cout << i + 1 << ". " << network[i].getName() << "\n";
            }
            int sIndex; std::cin >> sIndex; sIndex--;

            if (sIndex >= 0 && sIndex < static_cast<int>(network.size())) {
                network[sIndex].displayStatus();
                std::cout << "Enter the slot number you want to unplug: ";
                int slotNum; std::cin >> slotNum;
                
                // Manual removal applies the active tariff rate calculated for the current hour
                network[sIndex].undockVehicle(slotNum - 1, activeTariffRate);
            } else {
                std::cout << "[ERROR] Invalid station selection.\n";
            }
        }
    }

    std::cout << "\nShutting down simulation cores safely. Goodbye!\n";
    return 0;
}