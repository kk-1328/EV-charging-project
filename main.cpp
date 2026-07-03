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
    }
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
    double continuousPowerOutput; // in kW
    Vehicle* occupiedBy;          // Pointer to current vehicle (nullptr if empty)

public:
    ChargingPoint(int p_id, ChargerType t) : id(p_id), type(t), occupiedBy(nullptr) {
        continuousPowerOutput = (type == ChargerType::AC_SLOW) ? 7.0 : 50.0;
    }

    bool isAvailable() const { return occupiedBy == nullptr; }
    
    void plugVehicle(Vehicle* v) { occupiedBy = v; }
    void unplugVehicle() { occupiedBy = nullptr; }
    Vehicle* getVehicle() const { return occupiedBy; }
    double getPowerOutput() const { return continuousPowerOutput; }
    
    double getActualChargingSpeed() const {
    if (occupiedBy == nullptr) return 0.0;
    
    double stationPower = continuousPowerOutput;
    double vehicleMax = occupiedBy->getMaxChargingSpeed();
    
    // The bottleneck logic
    return (stationPower < vehicleMax) ? stationPower : vehicleMax;
}

    std::string getTypeString() const {
        return (type == ChargerType::AC_SLOW) ? "AC Slow (7kW)" : "DC Fast (50kW)";
    }
};