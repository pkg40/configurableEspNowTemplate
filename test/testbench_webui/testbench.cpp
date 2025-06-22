#include <iostream>
#include <fstream>
#include <configManager2.h>

void loadTestConfig(configManager2& configManager, const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "❌ Error: Cannot open " << filename << std::endl;
        return;
    }

    std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (!configManager.jsonStringToMap(&jsonString, true)) {
        std::cerr << "❌ JSON parsing failed!" << std::endl;
    } else {
        std::cout << "✅ Config Loaded Successfully!\n";
        configManager.printConfigToSerial();
    }
}

void modifyTestConfig(configManager2& configManager) {
    std::string section = "Server";
    std::string key = "ip";
    std::string newValue = "192.168.1.10";

    std::cout << "\n🔧 Modifying config: " << section << "." << key << " → " << newValue << std::endl;
    configManager.setValue(section, key, newValue);

    std::cout << "📝 New Configuration:\n";
    configManager.printConfigToSerial();
}

int main() {
    configManager2 configManager;

    std::cout << "\n🔍 Loading Test Config...\n";
    loadTestConfig(configManager, "test_config.json");

    std::cout << "\n💾 Running Modifications...\n";
    modifyTestConfig(configManager);

    return 0;
}