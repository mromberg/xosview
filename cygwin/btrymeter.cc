    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps))
        std::cerr << "GetSystemPowerStatus() failed." << std::endl;
    else {
        int batlife = sps.BatteryLifePercent;
        unsigned acstatus = sps.ACLineStatus; // 0/1 off/on line, 255 unknown
        bool high = sps.BatteryFlag & 1;
        bool low = sps.BatteryFlag & 2;
        bool crit = sps.BatteryFlag & 4;
        bool charging = sps.BatteryFlag & 8;
        bool nobat = sps.BatteryFlag & 128;
        bool unknownbat = sps.BatteryFlag & 255;
        //bool batsaver = sps.SystemStatusFlag; if bat saver is on.
        int batlifetime = sps.BatteryFullLifeTime; // -1 if on AC or unknown
        std::cout << "Bat life %: " << batlife << "\n"
                  << "AC status: " << acstatus << "\n"
                  << "high: " << high << "\n"
                  << "low: " << low << "\n"
                  << "crit: " << crit << "\n"
                  << "charging: " << charging << "\n"
                  << "nobat: " << nobat << "\n"
                  << "unknow bat: " << unknownbat << "\n"
                  << "bat life: " << batlifetime << " secs.\n";
    }
