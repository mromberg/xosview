//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015, 2016, 2018
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file contains code from the NetBSD project, which is covered
//    by the standard BSD license.
//  Dummy device ignore code by : David Cuka (dcuka@intgp1.ih.att.com)
//  The OpenBSD interrupt meter code was written by Oleg Safiullin
//    (form@vs.itam.nsc.ru).
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
#include "kernel.h"
#include "log.h"
#include "sctl.h"
#include "strutil.h"
#if defined(XOSVIEW_FREEBSD)
#include "cpumeter.h"
#endif

#include <map>

#include <unistd.h>
#include <fcntl.h>

#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
#include <dev/acpica/acpiio.h>
#include <machine/apm_bios.h>
#endif


#if defined(XOSVIEW_NETBSD)
#include <sys/envsys.h>
#include <prop/proplib.h>
#include <paths.h>
#endif

#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
#include <sys/sensors.h>
#endif



#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
static const char * const ACPIDEV = "/dev/acpi";
static const char * const APMDEV = "/dev/apm";
#endif



//  ---------------------- Sensor Meter stuff  -------------------------------

#if defined(__i386__) || defined(__x86_64__)
static size_t BSDGetCPUTemperatureMap(std::map<int, float> &temps,
  std::map<int, float> &tjmax) {
    temps.clear();
    tjmax.clear();
    size_t nbr = 0;
#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    (void)tjmax; // Avoid the warning
#endif

#if defined(XOSVIEW_NETBSD)
    // All kinds of sensors are read with libprop. We have to go through them
    // to find either Intel Core 2 or AMD ones. Actual temperature is in
    // cur-value and TjMax, if present, in critical-max.
    // Values are in microdegrees Kelvin.

    int fd;
    if ((fd = open(_PATH_SYSMON, O_RDONLY)) == -1) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return 0;  // this seems to happen occasionally, so only warn
    }

    prop_dictionary_t pdict;
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;
    if (prop_dictionary_count(pdict) == 0) {
        logProblem << "No sensors found" << std::endl;
        return 0;
    }
    const prop_object_iterator_t piter = prop_dictionary_iterator(pdict);
    if (!piter)
        logFatal << "Could not get sensor iterator" << std::endl;

    prop_object_t pobj = nullptr;
    while ((pobj = prop_object_iterator_next(piter))) {
        prop_array_t parray = static_cast<prop_array_t>(
            prop_dictionary_get_keysym(pdict,
              static_cast<prop_dictionary_keysym_t>(pobj)));
        if (prop_object_type(parray) != PROP_TYPE_ARRAY)
            continue;
        const std::string name(prop_dictionary_keysym_cstring_nocopy(
              static_cast<prop_dictionary_keysym_t>(pobj)));
        if (name.substr(0, 8) != "coretemp" && name.substr(0, 7) != "amdtemp")
            continue;

        const prop_object_iterator_t piter2 = prop_array_iterator(parray);
        if (!piter2)
            logFatal << "Could not get sensor iterator" << std::endl;

        std::istringstream is(name);
        size_t i = 0;
        is >> util::sink("*[!0-9]", true) >> i;

        while ((pobj = prop_object_iterator_next(piter2))) {
            prop_object_t pobj1 = prop_dictionary_get(
                static_cast<prop_dictionary_t>(pobj), "type");
            if (!pobj1)
                continue;
            if ((pobj1 = prop_dictionary_get(
                    static_cast<prop_dictionary_t>(pobj), "cur-value"))) {
                temps[i] = (prop_number_integer_value(
                      static_cast<prop_number_t>(pobj1)) / 1e6) - 273.15;
                nbr++;
            }
            const prop_object_t pobj2 = prop_dictionary_get(
                static_cast<prop_dictionary_t>(pobj), "critical-max");
            if (pobj2)
                tjmax[i] = (prop_number_integer_value(
                      static_cast<prop_number_t>(pobj2)) / 1e6) - 273.15;
        }
        prop_object_iterator_release(piter2);
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);

#elif defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    // All kinds of sensors are read with sysctl. We have to go through them
    // to find either Intel Core 2 or AMD ones.
    // Values are in microdegrees Kelvin.
    for (int dev = 0 ; dev < 1024 ; dev++) {  // go through all sensor devices
        SysCtl sc = { CTL_HW, HW_SENSORS, dev };
        struct sensordev sd;
        if (!sc.get(sd)) {
            if (errno == ENOENT)
                break;  // no more sensors
            if (errno == ENXIO)
                continue;  // no sensor with this mib
            logFatal << "sysctl hw.sensors." << dev << " failed" << std::endl;
        }
        if (std::string(sd.xname, 0, 3) != "cpu")
            continue;  // not CPU sensor
        std::istringstream is(sd.xname);
        int cpu = 0;
        is >> util::sink("*[!0-9]", true) >> cpu;

        // for each device, get temperature sensors.
        for (int i = 0 ; i < sd.maxnumt[SENSOR_TEMP] ; i++) {
            SysCtl sc_temp = { CTL_HW, HW_SENSORS, dev, SENSOR_TEMP, i };
            struct sensor sen;
            if (!sc_temp.get(sen))
                continue;  // no sensor on this core?
            if (sen.flags & SENSOR_FINVALID)
                continue;
            temps[cpu] = static_cast<float>(sen.value - 273150000) / 1000000.0;
            nbr++;
        }
    }
#else  // XOSVIEW_FREEBSD
    // Temperatures can be read with sysctl dev.cpu.%d.temperature on both
    // Intel Core 2 and AMD K8+ processors.
    // Values are in degrees Celsius (FreeBSD < 7.2) or in
    // 10*degrees Kelvin (FreeBSD >= 7.3).
    const size_t cpus = CPUMeter::countCPUs();

    for (size_t i = 0 ; i < cpus ; i++) {
        SysCtl sc_cput("dev.cpu." + std::to_string(i) + ".temperature");
        int val = 0;
        if (sc_cput.get(val)) {
            nbr++;
            temps[i] = (static_cast<float>(val) - 2732.0) / 10.0;
        }
        else
            logProblem << "sysctl " << sc_cput.id() << " failed" << std::endl;

        SysCtl sc_core("dev.cpu." + std::to_string(i) + ".coretemp.tjmax");
        if (sc_core.get(val))
            tjmax[i] = (static_cast<float>(val) - 2732.0) / 10.0;
        else
            logProblem << "sysctl " << sc_core.id() << " failed\n";
    }
#endif

    return nbr;
}


size_t BSDGetCPUTemperature(std::vector<float> &temps,
  std::vector<float> &tjmax) {
    std::map<int, float> tempM;
    std::map<int, float> tjmxM;
    size_t count = BSDGetCPUTemperatureMap(tempM, tjmxM);
    if (tempM.size() != count || tjmxM.size() != count)
        logFatal << "Internal core temp logic failure." << std::endl;

    temps.resize(count);
    tjmax.resize(count);

    // The std::map version that gets the data reads the cpu number
    // from a string.  The assumption is that this cpu number will
    // somehow be in the range [0-count].  We will populate the
    // vectors here and check this.  If it is not the case then
    // the train stops here.
    for (const auto &tm : tempM)
        if (tm.first >= 0 && tm.first < static_cast<int>(temps.size()))
            temps[tm.first] = tm.second;
        else
            logFatal << "Internal core temp logic failure." << std::endl;

    for (const auto &tj : tjmxM)
        if (tj.first >= 0 && tj.first < static_cast<int>(temps.size()))
            tjmax[tj.first] = tj.second;
        else
            logFatal << "Internal core temp logic failure." << std::endl;

    return count;
}

#endif


void BSDGetSensor(const std::string &name, const std::string &valname,
  float &value, std::string &unit) {

    logAssert(name.size() && valname.size()
      && value) << "'nullptr' passed to BSDGetSensor()." << std::endl;

#if defined(XOSVIEW_NETBSD)
    // Adapted from envstat.
    // All kinds of sensors are read with libprop. Specific device and value
    // can be asked for. Values are transformed to suitable units.

    int fd = -1;
    if ((fd = open(_PATH_SYSMON, O_RDONLY)) == -1) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return;  // this seems to happen occasionally, so only warn
    }
    prop_dictionary_t pdict;
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if (prop_dictionary_count(pdict) == 0) {
        logProblem << "No sensors found" << std::endl;
        return;
    }

    prop_object_t pobj = prop_dictionary_get(pdict, name.c_str());
    if (prop_object_type(pobj) != PROP_TYPE_ARRAY)
        logFatal << "Device " << name << " does not exist" << std::endl;

    const prop_object_iterator_t piter = prop_array_iterator(
        static_cast<prop_array_t>(pobj));
    if (!piter)
        logFatal << "Could not get sensor iterator" << std::endl;

    while ((pobj = prop_object_iterator_next(piter))) {
        prop_object_t pobj1 = prop_dictionary_get(
            static_cast<prop_dictionary_t>(pobj), "type");
        if (!pobj1)
            continue;

        const std::string ptype(prop_string_cstring_nocopy(
              static_cast<prop_string_t>(pobj1)));

        if (ptype == "Indicator" || ptype == "Battery" || ptype == "Drive")
            continue;  // these are string values

        int val = 0;
        if ((pobj1 = prop_dictionary_get(static_cast<prop_dictionary_t>(pobj),
              valname.c_str())))
            val = prop_number_integer_value(static_cast<prop_number_t>(pobj1));
        else
            logFatal << "Value " << valname << " does not exist\n";

        // Assign value and unit based on the following table:
        // key : {X, Y, Unit}
        // Where value = (float)val / X + Y
        const std::map<std::string, std::tuple<double, double, std::string>>
            umap = {
            {"Temperature", {1e6, -273.15, "C"}},
            {"Fan", {1, 0, "RPM"}},
            {"Integer", {1, 0, ""}},
            {"Voltage", {1e6, 0, "V"}},
            {"Ampere hour", {1e6, 0, "Ah"}},
            {"Ampere", {1e6, 0, "A"}},
            {"Watt hour", {1e6, 0, "Wh"}},
            {"Watts", {1e6, 0, "W"}},
            {"Ohms", {1e6, 0, "Ohm"}}
        };

        const auto &it = umap.find(ptype);
        if (it != umap.end()) {
            const auto &tpl = it->second;
            value = static_cast<double>(val) / std::get<0>(tpl)
                + std::get<1>(tpl);
            unit = std::get<2>(tpl);
        }
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);

#else  // everything not XOSVIEW_NETBSD

#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
    // FreeBSD has no sensor framework, but ACPI thermal zones might work.
    // They are readable through sysctl (also works in Dragonfly).
    // Values are in 10 * degrees Kelvin.
    if (name == "tz") {
        int val = 0;
        SysCtl sc_thrm("hw.acpi.thermal.tz." + valname);
        if (!sc_thrm.get(val))
            logFatal << "sysctl " << sc_thrm.id() << " failed" << std::endl;
        value = (static_cast<float>(val) - 2732.0) / 10.0;
        if (!unit.empty())
            unit = "uC";
        return;
    }
    // If Dragonfly and tzN specified, return. Otherwise, fall through.
#endif
#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    // Adapted from systat.
    // All kinds of sensors are read with sysctl. We have to go through them
    // to find the required device and value. Parameter 'name' is the device
    // name and 'valname' consists of type and sensor index (e.g. it0.temp1).
    //  Values are transformed to suitable units.

    for (int dev = 0 ; dev < 1024 ; dev++) {  // go through all sensor devices
        struct sensordev sd;
        SysCtl sc_dev = { CTL_HW, HW_SENSORS, dev };
        if (!sc_dev.get(sd)) {
            if (errno == ENOENT)
                break;  // no more devices
            if (errno == ENXIO)
                continue;  // no device with this mib
            logFatal << "sysctl hw.sensors." << dev << " failed" << std::endl;
        }

        if (std::string(sd.xname) != name)
            continue;

        for (int t = 0 ; t < SENSOR_MAX_TYPES ; t++) {
            const std::string stype(sensor_type_s[t]);
            if (stype != valname)
                continue;  // wrong type

            std::istringstream is(valname);
            int index;
            is >> util::sink("*[!0-9]", true) >> index;
            if (index < sd.maxnumt[t]) {
                SysCtl sc_sen = { CTL_HW, HW_SENSORS, dev, t, index };
                struct sensor sen;
                if (!sc_sen.get(sen)) {
                    if (errno != ENOENT)
                        logFatal << "sysctl hw.sensors."
                                 << dev << "." << t << "." << index
                                 << " failed" << std::endl;

                    continue;  // no more sensors
                }

                if (sen.flags & SENSOR_FINVALID)
                    continue;

                // Assign value and unit based on the following table:
                // key : {X, Y, Unit}
                // Where value = (float)val / X + Y
                const std::map<int, std::tuple<double, double, std::string>>
                    umap = {
                    {SENSOR_TEMP, std::make_tuple(1e6, -273.15, "C")},
                    {SENSOR_FANRPM, std::make_tuple(1, 0, "RPM")},
                    {SENSOR_VOLTS_DC, std::make_tuple(1e6, 0, "Vdc")},
                    {SENSOR_VOLTS_AC, std::make_tuple(1e6, 0, "Vac")},
                    {SENSOR_OHMS, std::make_tuple(1e6, 0, "Ohm")},
                    {SENSOR_WATTS, std::make_tuple(1e6, 0, "W")},
                    {SENSOR_AMPS, std::make_tuple(1e6, 0, "A")},
                    {SENSOR_AMPHOUR, std::make_tuple(1e6, 0, "Ah")},
                    {SENSOR_WATTHOUR, std::make_tuple(1e6, 0, "Wh")},
                    {SENSOR_PERCENT, std::make_tuple(1e3, 0, "%")},
                    {SENSOR_LUX, std::make_tuple(1e6, 0, "lx")},
                    {SENSOR_TIMEDELTA, std::make_tuple(1e9, 0, "s")},
#if defined(XOSVIEW_OPENBSD)
                    {SENSOR_HUMIDITY, {1e3, 0, "%"}},
                    {SENSOR_FREQ, {1e6, 0, "Hz"}},
                    {SENSOR_ANGLE, {1e6, 0, "d"}},
                    {SENSOR_DISTANCE, {1e6, 0, "m"}},
                    {SENSOR_PRESSURE, {1e3, 0, "Pa"}},
                    {SENSOR_ACCEL, {1e6, 0, "m/s^2"}},
#endif
                };

                const auto &it = umap.find(t);
                if (it != umap.end()) {
                    const auto &tpl = it->second;
                    value = static_cast<double>(sen.value) / std::get<0>(tpl)
                        + std::get<1>(tpl);
                    unit = std::get<2>(tpl);
                }
                else { // default
                    value = static_cast<double>(sen.value);
                    unit.clear();
                }
            }
        }
    }
#endif
#endif
}


//  ---------------------- Battery Meter stuff ---------------------------------

bool BSDHasBattery(void) {
#if defined(XOSVIEW_NETBSD)

    int fd = -1;
    if ((fd = open(_PATH_SYSMON, O_RDONLY)) == -1)
        return false;

    prop_dictionary_t pdict;
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if (prop_dictionary_count(pdict) == 0)
        return false;

    // just check for 1st battery.
    const prop_object_t pobj = prop_dictionary_get(pdict, "acpibat0");
    if (prop_object_type(pobj) != PROP_TYPE_ARRAY)
        return false;

    return true;

#elif defined(XOSVIEW_OPENBSD)
    // check if we can get full capacity of the 1st battery
    float val = -1.0;
    std::string emptyStr;
    BSDGetSensor("acpibat0", "amphour0", val, emptyStr);
    if (val < 0)
        return false;
    return true;
#else // XOSVIEW_FREEBSD || XOSVIEW_DFBSD
    int fd;
    if ((fd = open(ACPIDEV, O_RDONLY)) == -1) {
        // No ACPI -> try APM
        if ((fd = open(APMDEV, O_RDONLY)) == -1)
            return false;

        struct apm_info aip;
        if (ioctl(fd, APMIO_GETINFO, &aip) == -1)
            return false;
        if (close(fd) == -1)
            logFatal << "Could not close " << APMDEV << std::endl;
        if (aip.ai_batt_stat == 0xff || aip.ai_batt_life == 0xff)
            return false;
        return true;
    }

    union acpi_battery_ioctl_arg battio;
    battio.unit = ACPI_BATTERY_ALL_UNITS;
    if (ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1)
        return false;
    if (close(fd) == -1)
        logFatal << "Could not close " << ACPIDEV << std::endl;

    return battio.battinfo.state != ACPI_BATT_STAT_NOT_PRESENT;
#endif
}


void BSDGetBatteryInfo(int &remaining, unsigned int &state) {
    state = XOSVIEW_BATT_NONE;
#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_OPENBSD)

#if defined(XOSVIEW_NETBSD)
    // Again adapted from envstat.
    // All kinds of sensors are read with libprop. We have to go through them
    // to find the batteries. We need capacity, charge, presence, charging
    // status and discharge rate for each battery for the calculations.
    // For simplicity, assume all batteries have the same
    // charge/discharge status.
    int batteries = 0;

    int fd = -1;
    if ((fd = open(_PATH_SYSMON, O_RDONLY)) == -1) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return;  // this seems to happen occasionally, so only warn
    }

    prop_dictionary_t pdict;
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;
    if (prop_dictionary_count(pdict) == 0) {
        logProblem << "No sensors found" << std::endl;
        return;
    }

    const prop_object_iterator_t piter = prop_dictionary_iterator(pdict);
    if (!piter)
        logFatal << "Could not get sensor iterator" << std::endl;

    // This snippet is repeated often below.  Macro it here.
    const auto setIfFound = [](prop_dictionary_t d, const char *k, int &v) {
        auto pobj = prop_dictionary_get(d, k);
        if (pobj) {
            v = prop_number_integer_value(static_cast<prop_number_t>(pobj));
            return true;
        }
        return false;
    };

    // Sum of all batteries.
    int total_capacity = 0, total_charge = 0, total_low = 0, total_crit = 0;

    prop_object_t pobj = nullptr;
    while ((pobj = prop_object_iterator_next(piter))) {
        std::string name(prop_dictionary_keysym_cstring_nocopy(
              static_cast<prop_dictionary_keysym_t>(pobj)));
        if (name.substr(0, 7) != "acpibat")
            continue;

        const prop_array_t parray = static_cast<prop_array_t>(
            prop_dictionary_get_keysym(pdict,
              static_cast<prop_dictionary_keysym_t>(pobj)));
        if (prop_object_type(parray) != PROP_TYPE_ARRAY)
            continue;

        const prop_object_iterator_t piter2 = prop_array_iterator(parray);
        if (!piter2)
            logFatal << "Could not get sensor iterator" << std::endl;

        // For each battery find.
        int present = 0, capacity = 0, charge = 0, low = 0, crit = 0;

        while ((pobj = prop_object_iterator_next(piter2))) {
            const prop_dictionary_t pdict =
                static_cast<prop_dictionary_t>(pobj);
            prop_object_t pobj1 = prop_dictionary_get(pdict, "state");
            if (!pobj1)
                continue;

            if (prop_string_equals_cstring(static_cast<prop_string_t>(pobj1),
                "invalid")
              || prop_string_equals_cstring(static_cast<prop_string_t>(pobj1),
                "unknown"))
                continue; // skip sensors without valid data
            if (!(pobj1 = prop_dictionary_get(pdict, "description")))
                continue;

            name = prop_string_cstring_nocopy(
                static_cast<prop_string_t>(pobj1));

            if (name.substr(0, 7) == "present") // is battery present
                setIfFound(pdict, "cur-value", present);
            else if (name.substr(0, 10) == "design cap") // get full capacity
                setIfFound(pdict, "cur-value", capacity);
            else if (name.substr(0, 7) == "charge") {
                // get present charge, low and critical levels
                setIfFound(pdict, "cur-value", charge);
                setIfFound(pdict, "warning-capacity", low);
                setIfFound(pdict, "critical-capacity", crit);
            }
            else if (name.substr(0, 8) == "charging") {
                // charging or not?
                int unused = 0;
                if (setIfFound(pdict, "cur-value", unused))
                    state |= XOSVIEW_BATT_CHARGING;
            }
            else if (name.substr(0, 14) == "discharge rate") {
                // discharging or not?
                int unused = 0;
                if (setIfFound(pdict, "cur-value", unused))
                    state |= XOSVIEW_BATT_DISCHARGING;
            }
        }

        if (present) {
            total_capacity += capacity;
            total_charge += charge;
            total_low += low;
            total_crit += crit;
            batteries++;
        }
        prop_object_iterator_release(piter2);
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);

#else // XOSVIEW_OPENBSD

    float total_capacity = 0, total_charge = 0, total_low = 0, total_crit = 0;

    int batteries = 0;
    while (batteries < 1024) {
        const std::string battery = "acpibat" + util::repr(batteries);
        float val = -1.0;
        std::string emptyStr;
        BSDGetSensor(battery, "amphour0", val, emptyStr); // full capacity

        if (val < 0) // no more batteries
            break;

        batteries++;
        total_capacity += val;
        emptyStr.clear();
        BSDGetSensor(battery, "amphour1", val, emptyStr); // warning capacity
        total_low += val;
        emptyStr.clear();
        BSDGetSensor(battery, "amphour2", val, emptyStr); // low capacity
        total_crit += val;
        emptyStr.clear();
        BSDGetSensor(battery, "amphour3", val, emptyStr); // remaining
        total_charge += val;
        emptyStr.clear();
        BSDGetSensor(battery, "raw0", val, emptyStr); // state
        if (static_cast<int>(val) == 1)
            state |= XOSVIEW_BATT_DISCHARGING;
        else if (static_cast<int>(val) == 2)
            state |= XOSVIEW_BATT_CHARGING;
        // there's also 0 state for idle/full
    }
#endif
    if (batteries == 0) { // all batteries are off
        state = XOSVIEW_BATT_NONE;
        remaining = 0;
        return;
    }
    remaining = 100 * total_charge / total_capacity;
    if (!(state & XOSVIEW_BATT_CHARGING) &&
      !(state & XOSVIEW_BATT_DISCHARGING))
        state |= XOSVIEW_BATT_FULL;  // full when not charging nor discharging
    if (total_capacity < total_low)
        state |= XOSVIEW_BATT_LOW;
    if (total_capacity < total_crit)
        state |= XOSVIEW_BATT_CRITICAL;
#else // XOSVIEW_FREEBSD || XOSVIEW_DFBSD
    // Adapted from acpiconf and APM.
    int fd;
    if ((fd = open(ACPIDEV, O_RDONLY)) == -1) {
        // No ACPI -> try APM.
        if ((fd = open(APMDEV, O_RDONLY)) == -1)
            logFatal << "could not open " << ACPIDEV << " or " << APMDEV
                     << std::endl;

        struct apm_info aip;
        if (ioctl(fd, APMIO_GETINFO, &aip) == -1)
            logFatal << "failed to get APM battery info" << std::endl;
        if (close(fd) == -1)
            logFatal << "Could not close " << APMDEV << std::endl;
        if (aip.ai_batt_life <= 100)
            remaining = aip.ai_batt_life; // only 0-100 are valid values
        else
            remaining = 0;
        if (aip.ai_batt_stat == 0)
            state |= XOSVIEW_BATT_FULL;
        else if (aip.ai_batt_stat == 1)
            state |= XOSVIEW_BATT_LOW;
        else if (aip.ai_batt_stat == 2)
            state |= XOSVIEW_BATT_CRITICAL;
        else if (aip.ai_batt_stat == 3)
            state |= XOSVIEW_BATT_CHARGING;
        else
            state = XOSVIEW_BATT_NONE;
    }
    else { // ACPI
        union acpi_battery_ioctl_arg battio;
        battio.unit = ACPI_BATTERY_ALL_UNITS;
        if (ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1)
            logFatal << "failed to get ACPI battery info" << std::endl;
        if (close(fd) == -1)
            logFatal << "Could not close " << ACPIDEV << std::endl;

        remaining = battio.battinfo.cap;
        if (battio.battinfo.state != ACPI_BATT_STAT_NOT_PRESENT) {
            if (battio.battinfo.state == 0)
                state |= XOSVIEW_BATT_FULL;
            if (battio.battinfo.state & ACPI_BATT_STAT_CRITICAL)
                state |= XOSVIEW_BATT_CRITICAL;
            if (battio.battinfo.state & ACPI_BATT_STAT_DISCHARG)
                state |= XOSVIEW_BATT_DISCHARGING;
            if (battio.battinfo.state & ACPI_BATT_STAT_CHARGING)
                state |= XOSVIEW_BATT_CHARGING;
        }
    }
#endif
}
