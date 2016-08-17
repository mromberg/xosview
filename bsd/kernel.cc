//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015, 2016
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
#include "strutil.h"
#include "sctl.h"

#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include <sys/param.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>


#if defined(XOSVIEW_DFBSD)
#define _KERNEL_STRUCTURES
#include <kinfo.h>
#endif


#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
#include <net/if.h>
#include <net/if_var.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <dev/acpica/acpiio.h>
#include <machine/apm_bios.h>
#include <net/if_mib.h>
#endif


#if defined(XOSVIEW_NETBSD)
#include <sys/sched.h>
#include <sys/iostat.h>
#include <sys/envsys.h>
#include <prop/proplib.h>
#include <paths.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <machine/int_fmtio.h>
#endif


#if defined(XOSVIEW_OPENBSD)
#include <sys/sched.h>
#include <sys/disk.h>
#if defined(HAVE_SYS_DKSTAT_H)
#include <sys/dkstat.h>
#elif defined(HAVE_SYS_SCHED_H)
#include <sys/sched.h>
#endif
#include <sys/mount.h>
#include <net/route.h>
#include <net/if_dl.h>
#endif


#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
#include <sys/sensors.h>
#endif

#if defined(HAVE_DEVSTAT)
#include <devstat.h>
#endif

#if defined(HAVE_UVM)
#include <sys/device.h>
#include <uvm/uvm_extern.h>
#else
#include <sys/vmmeter.h>
#endif

#if defined(HAVE_SWAPCTL)
#include <sys/swap.h>
#endif


// Number of elements in a static array.
#define ASIZE(ar) (sizeof(ar) / sizeof(ar[0]))


#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
static const char * const ACPIDEV = "/dev/acpi";
static const char * const APMDEV = "/dev/apm";
#endif


#if defined(XOSVIEW_FREEBSD)
static size_t BSDCountCpus(void) {

    static SysCtl ncpu_sc("hw.ncpu");
    static int cpus = -1;

    if (cpus == -1) {
        if (!ncpu_sc.get(cpus))
            logFatal << "sysctl(" << ncpu_sc.id() << "failed." << std::endl;
    }

    return cpus;
}
#endif


//  ---------------------- Sensor Meter stuff  ---------------------------------

#if defined(__i386__) || defined(__x86_64__)
static unsigned int BSDGetCPUTemperatureMap(std::map<int, float> &temps,
  std::map<int, float> &tjmax) {
    temps.clear();
    tjmax.clear();
    unsigned int nbr = 0;
#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    (void)tjmax; // Avoid the warning
#endif
#if defined(XOSVIEW_NETBSD)
    // All kinds of sensors are read with libprop. We have to go through them
    // to find either Intel Core 2 or AMD ones. Actual temperature is in
    // cur-value and TjMax, if present, in critical-max.
    // Values are in microdegrees Kelvin.
    int fd;
    const char *name = NULL;
    prop_dictionary_t pdict;
    prop_object_t pobj, pobj1, pobj2;
    prop_object_iterator_t piter, piter2;
    prop_array_t parray;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 ) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return 0;  // this seems to happen occasionally, so only warn
    }
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if (prop_dictionary_count(pdict) == 0) {
        logProblem << "No sensors found" << std::endl;
        return 0;
    }
    if ( !(piter = prop_dictionary_iterator(pdict)) )
        logFatal << "Could not get sensor iterator" << std::endl;

    while ( (pobj = prop_object_iterator_next(piter)) ) {
        parray = (prop_array_t)prop_dictionary_get_keysym(pdict,
          (prop_dictionary_keysym_t)pobj);
        if (prop_object_type(parray) != PROP_TYPE_ARRAY)
            continue;
        name = prop_dictionary_keysym_cstring_nocopy(
            (prop_dictionary_keysym_t)pobj);
        if (std::string(name, 0, 8) != "coretemp"
          && std::string(name, 0, 7) != "amdtemp")
            continue;
        if ( !(piter2 = prop_array_iterator(parray)) )
            logFatal << "Could not get sensor iterator" << std::endl;

        int i = 0;
        std::istringstream is(name);
        is >> util::sink("*[!0-9]", true) >> i;
        while ( (pobj = prop_object_iterator_next(piter2)) ) {
            if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "type")) )
                continue;
            if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "cur-value")) ) {
                temps[i] = (prop_number_integer_value((prop_number_t)pobj1)
                  / 1000000.0) - 273.15;
                nbr++;
            }
            if ( (pobj2 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "critical-max")))
                tjmax[i] = (prop_number_integer_value(
                      (prop_number_t)pobj2) / 1000000.0) - 273.15;
        }
        prop_object_iterator_release(piter2);
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);
#else  /* XOSVIEW_NETBSD */
    int val = 0;
    size_t size = sizeof(val);

#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    // All kinds of sensors are read with sysctl. We have to go through them
    // to find either Intel Core 2 or AMD ones.
    // Values are in microdegrees Kelvin.
    struct sensordev sd;
    struct sensor s;
    int cpu = 0;
    int mib_sen[] = { CTL_HW, HW_SENSORS, 0, 0, 0 };

    for (int dev = 0; dev < 1024; dev++) {  // go through all sensor devices
        mib_sen[2] = dev;
        size = sizeof(sd);
        if ( sysctl(mib_sen, 3, &sd, &size, NULL, 0) < 0 ) {
            if (errno == ENOENT)
                break;  // no more sensors
            if (errno == ENXIO)
                continue;  // no sensor with this mib
            logFatal << "sysctl hw.sensors." << dev << " failed" << std::endl;
        }
        if (std::string(sd.xname, 0, 3) != "cpu")
            continue;  // not CPU sensor
        std::istringstream is(sd.xname);
        is >> util::sink("*[!0-9]", true) >> cpu;

        mib_sen[3] = SENSOR_TEMP;  // for each device, get temperature sensors
        for (int i = 0; i < sd.maxnumt[SENSOR_TEMP]; i++) {
            mib_sen[4] = i;
            size = sizeof(s);
            if ( sysctl(mib_sen, 5, &s, &size, NULL, 0) < 0 )
                continue;  // no sensor on this core?
            if (s.flags & SENSOR_FINVALID)
                continue;
            temps[cpu] = (float)(s.value - 273150000) / 1000000.0;
            nbr++;
        }
    }
#else  /* XOSVIEW_FREEBSD */
    // Temperatures can be read with sysctl dev.cpu.%d.temperature on both
    // Intel Core 2 and AMD K8+ processors.
    // Values are in degrees Celsius (FreeBSD < 7.2) or in
    // 10*degrees Kelvin (FreeBSD >= 7.3).
    std::string name;
    int cpus = BSDCountCpus();
    for (int i = 0; i < cpus; i++) {
        name = "dev.cpu." + util::repr(i) + ".temperature";
        if ( sysctlbyname(name.c_str(), &val, &size, NULL, 0) == 0) {
            nbr++;
#if __FreeBSD_version >= 702106
            temps[i] = ((float)val - 2732.0) / 10.0;
#else
            temps[i] = (float)val;
#endif
        }
        else
            logProblem << "sysctl " << name << " failed" << std::endl;

        name = "dev.cpu." + util::repr(i) + ".coretemp.tjmax";
        if ( sysctlbyname(name.c_str(), &val, &size, NULL, 0) == 0 )
#if __FreeBSD_version >= 702106
            tjmax[i] = ((float)val - 2732.0) / 10.0;
#else
            tjmax[i] = (float)val;
#endif
        else
            logProblem << "sysctl " << name << " failed\n";
    }
#endif
#endif
    return nbr;
}


unsigned int BSDGetCPUTemperature(std::vector<float> &temps,
  std::vector<float> &tjmax) {
    std::map<int, float> tempM;
    std::map<int, float> tjmxM;
    unsigned int count = BSDGetCPUTemperatureMap(tempM, tjmxM);
    if (tempM.size() != count || tjmxM.size() != count)
        logFatal << "Internal core temp logic failure." << std::endl;

    temps.resize(count);
    tjmax.resize(count);

    // The std::map version that gets the data reads the cpu number
    // from a string.  The assumption is that this cpu number will
    // somehow be in the range [0-count].  We will populate the
    // vectors here and check this.  If it is not the case then
    // the train stops here.
    for (std::map<int,float>::iterator it=tempM.begin(); it!=tempM.end(); ++it)
        if (it->first >= 0 && it->first < (int)temps.size())
            temps[it->first] = it->second;
        else
            logFatal << "Internal core temp logic failure." << std::endl;

    for (std::map<int,float>::iterator it=tjmxM.begin(); it!=tjmxM.end(); ++it)
        if (it->first >= 0 && it->first < (int)temps.size())
            tjmax[it->first] = it->second;
        else
            logFatal << "Internal core temp logic failure." << std::endl;

    return count;
}

#endif


void BSDGetSensor(const std::string &name, const std::string &valname,
  float &value, std::string &unit) {

    logAssert(name.size() && valname.size()
      && value) << "'NULL' pointer passed to BSDGetSensor()." << std::endl;

#if defined(XOSVIEW_NETBSD)
    /* Adapted from envstat. */
    // All kinds of sensors are read with libprop. Specific device and value
    // can be asked for. Values are transformed to suitable units.
    int fd, val = 0;
    prop_dictionary_t pdict;
    prop_object_t pobj, pobj1;
    prop_object_iterator_t piter;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 ) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return;  // this seems to happen occasionally, so only warn
    }
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if (prop_dictionary_count(pdict) == 0) {
        logProblem << "No sensors found" << std::endl;
        return;
    }
    pobj = prop_dictionary_get(pdict, name.c_str());
    if (prop_object_type(pobj) != PROP_TYPE_ARRAY)
        logFatal << "Device " << name << " does not exist" << std::endl;

    if ( !(piter = prop_array_iterator((prop_array_t)pobj)) )
        logFatal << "Could not get sensor iterator" << std::endl;

    while ( (pobj = prop_object_iterator_next(piter)) ) {
        if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj, "type")) )
            continue;

        std::string ptype(prop_string_cstring_nocopy((prop_string_t)pobj1));

        if ( ptype == "Indicator" || ptype == "Battery"
          || ptype == "Drive" )
            continue;  // these are string values
        if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
              valname.c_str())) )
            val = prop_number_integer_value((prop_number_t)pobj1);
        else
            logFatal << "Value " << valname << " does not exist\n";
        if ( ptype == "Temperature" ) {
            value = (val / 1000000.0) - 273.15;  // temps are in microkelvins
            unit = "\260C";
        }
        else if ( ptype == "Fan") {
            value = (float)val;                  // plain integer value
            unit = "RPM";
        }
        else if ( ptype == "Integer" )
            value = (float)val;                  // plain integer value
        else if ( ptype == "Voltage" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "V";
        }
        else if ( ptype == "Ampere hour" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Ah";
        }
        else if ( ptype == "Ampere" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "A";
        }
        else if ( ptype == "Watt hour" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Wh";
        }
        else if ( ptype == "Watts" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "W";
        }
        else if ( ptype == "Ohms" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Ohm";
        }
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);
#else  /* XOSVIEW_NETBSD */
    size_t size;
    std::string dummy;
#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
    // FreeBSD has no sensor framework, but ACPI thermal zones might work.
    // They are readable through sysctl (also works in Dragonfly).
    // Values are in 10 * degrees Kelvin.
    if ( name == "tz" ) {
        int val = 0;
        size = sizeof(val);
        dummy = "hw.acpi.thermal." + name + "." + valname;
        if ( sysctlbyname(dummy.c_str(), &val, &size, NULL, 0) < 0 )
            logFatal << "sysctl " << dummy << " failed" << std::endl;
        value = ((float)val - 2732.0) / 10.0;
        if (unit.size())
            unit = "\260C";
        return;
    }
    // If Dragonfly and tzN specified, return. Otherwise, fall through.
#endif
#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    /* Adapted from systat. */
    // All kinds of sensors are read with sysctl. We have to go through them
    // to find the required device and value. Parameter 'name' is the device
    // name and 'valname' consists of type and sensor index (e.g. it0.temp1).
    //  Values are transformed to suitable units.
    int index = -1;
    struct sensordev sd;
    struct sensor s;
    int mib_sen[] = { CTL_HW, HW_SENSORS, 0, 0, 0 };

    for (int dev = 0; dev < 1024; dev++) {  // go through all sensor devices
        mib_sen[2] = dev;
        size = sizeof(sd);
        if ( sysctl(mib_sen, 3, &sd, &size, NULL, 0) < 0 ) {
            if (errno == ENOENT)
                break;  // no more devices
            if (errno == ENXIO)
                continue;  // no device with this mib
            logFatal << "sysctl hw.sensors." << dev << " failed" << std::endl;
        }
        std::string sname(name);
        if (std::string(sd.xname).substr(0, sname.size()) == name)
            continue;  // sensor name does not match

        for (int t = 0; t < SENSOR_MAX_TYPES; t++) {
            std::string stype_s(sensor_type_s[t]);
            if (stype_s != valname.substr(0, stype_s.size()))
                continue;  // wrong type
            mib_sen[3] = t;
            std::istringstream is(valname);
            is >> util::sink("*[!0-9]", true) >> index;
            if (index < sd.maxnumt[t]) {
                mib_sen[4] = index;
                size = sizeof(s);
                if ( sysctl(mib_sen, 5, &s, &size, NULL, 0) < 0 ) {
                    if (errno != ENOENT)
                        logFatal << "sysctl hw.sensors."
                                 << dev << "." << t << "." << index
                                 << " failed" << std::endl;

                    continue;  // no more sensors
                }
                if (s.flags & SENSOR_FINVALID)
                    continue;
                switch (t) {
                case SENSOR_TEMP:
                    value = (float)(s.value - 273150000) / 1000000.0;
                    unit = "\260C";
                    break;
                case SENSOR_FANRPM:
                    value = (float)s.value;
                    unit = "RPM";
                    break;
                case SENSOR_VOLTS_DC:
                case SENSOR_VOLTS_AC:
                    value = (float)s.value / 1000000.0;
                    unit = "V";
                    break;
                case SENSOR_OHMS:
                    value = (float)s.value;
                    unit = "Ohm";
                    break;
                case SENSOR_WATTS:
                    value = (float)s.value / 1000000.0;
                    unit = "W";
                    break;
                case SENSOR_AMPS:
                    value = (float)s.value / 1000000.0;
                    unit = "A";
                    break;
                case SENSOR_WATTHOUR:
                    value = (float)s.value / 1000000.0;
                    unit = "Wh";
                    break;
                case SENSOR_AMPHOUR:
                    value = (float)s.value / 1000000.0;
                    unit = "Ah";
                    break;
                case SENSOR_PERCENT:
                    value = (float)s.value / 1000.0;
                    unit = "%";
                    break;
                case SENSOR_LUX:
                    value = (float)s.value / 1000000.0;
                    unit = "lx";
                    break;
                case SENSOR_TIMEDELTA:
                    value = (float)s.value / 1000000000.0;
                    unit = "s";
                    break;
#if defined(XOSVIEW_OPENBSD)
                case SENSOR_HUMIDITY:
                    value = (float)s.value / 1000.0;
                    unit = "%";
                    break;
                case SENSOR_FREQ:
                    value = (float)s.value / 1000000.0;
                    unit = "Hz";
                    break;
                case SENSOR_ANGLE:
                    value = (float)s.value / 1000000.0;
                    unit = "\260";
                    break;
#if OpenBSD > 201211
                case SENSOR_DISTANCE:
                    value = (float)s.value / 1000000.0;
                    unit = "m";
                    break;
                case SENSOR_PRESSURE:
                    value = (float)s.value / 1000.0;
                    unit = "Pa";
                    break;
                case SENSOR_ACCEL:
                    value = (float)s.value / 1000000.0;
                    unit = "m\\/s\262"; // m/sÅ¬Å≤
                    break;
#endif
#endif
                case SENSOR_INDICATOR:
                case SENSOR_INTEGER:
                case SENSOR_DRIVE:
                default:
                    value = (float)s.value;
                    break;
                }
            }
        }
    }
#endif
#endif
}


//  ---------------------- Battery Meter stuff ---------------------------------

bool BSDHasBattery() {
#if defined(XOSVIEW_NETBSD)
    int fd;
    prop_dictionary_t pdict;
    prop_object_t pobj;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 )
        return false;
    if ( prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict) )
        logFatal << "Could not get sensor dictionary" << std::endl;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if ( prop_dictionary_count(pdict) == 0 )
        return false;
    pobj = prop_dictionary_get(pdict, "acpibat0"); // just check for 1st battery
    if ( prop_object_type(pobj) != PROP_TYPE_ARRAY )
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
    if ( (fd = open(ACPIDEV, O_RDONLY)) == -1 ) {
        // No ACPI -> try APM
        if ( (fd = open(APMDEV, O_RDONLY)) == -1 )
            return false;
        struct apm_info aip;
        if ( ioctl(fd, APMIO_GETINFO, &aip) == -1 )
            return false;
        if ( close(fd) == -1 )
            logFatal << "Could not close " << APMDEV << std::endl;
        if (aip.ai_batt_stat == 0xff || aip.ai_batt_life == 0xff)
            return false;
        return true;
    }

    union acpi_battery_ioctl_arg battio;
    battio.unit = ACPI_BATTERY_ALL_UNITS;
    if ( ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1 )
        return false;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << ACPIDEV << std::endl;
    return ( battio.battinfo.state != ACPI_BATT_STAT_NOT_PRESENT );
#endif
}


void BSDGetBatteryInfo(int &remaining, unsigned int &state) {
    state = XOSVIEW_BATT_NONE;
#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_OPENBSD)
    int batteries = 0;
#if defined(XOSVIEW_NETBSD)
    /* Again adapted from envstat. */
    // All kinds of sensors are read with libprop. We have to go through them
    // to find the batteries. We need capacity, charge, presence, charging
    // status and discharge rate for each battery for the calculations.
    // For simplicity, assume all batteries have the same
    // charge/discharge status.
    int fd;
    int total_capacity = 0, total_charge = 0, total_low = 0, total_crit = 0;
    const char *name = NULL;
    prop_dictionary_t pdict;
    prop_object_t pobj, pobj1;
    prop_object_iterator_t piter, piter2;
    prop_array_t parray;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 ) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return;  // this seems to happen occasionally, so only warn
    }
    if ( prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict) )
        logFatal << "Could not get sensor dictionary" << std::endl;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if ( prop_dictionary_count(pdict) == 0 ) {
        logProblem << "No sensors found" << std::endl;
        return;
    }
    if ( !(piter = prop_dictionary_iterator(pdict)) )
        logFatal << "Could not get sensor iterator" << std::endl;

    while ( (pobj = prop_object_iterator_next(piter)) ) {
        int present = 0, capacity = 0, charge = 0, low = 0, crit = 0;
        name = prop_dictionary_keysym_cstring_nocopy(
            (prop_dictionary_keysym_t)pobj);
        if ( std::string(name, 0, 7) != "acpibat" )
            continue;
        parray = (prop_array_t)prop_dictionary_get_keysym(pdict,
          (prop_dictionary_keysym_t)pobj);
        if ( prop_object_type(parray) != PROP_TYPE_ARRAY )
            continue;
        if ( !(piter2 = prop_array_iterator(parray)) )
            logFatal << "Could not get sensor iterator" << std::endl;

        while ( (pobj = prop_object_iterator_next(piter2)) ) {
            if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "state")) )
                continue;
            if ( prop_string_equals_cstring((prop_string_t)pobj1, "invalid")
              || prop_string_equals_cstring((prop_string_t)pobj1, "unknown") )
                continue; // skip sensors without valid data
            if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "description")) )
                continue;
            name = prop_string_cstring_nocopy((prop_string_t)pobj1);
            if ( std::string(name, 0, 7) == "present" ) { // is battery present
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    present = prop_number_integer_value((prop_number_t)pobj1);
            }
            else if ( std::string(name, 0, 10) == "design cap" ) {
                // get full capacity
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    capacity = prop_number_integer_value((prop_number_t)pobj1);
            }
            else if ( std::string(name, 0, 7) == "charge" ) {
                // get present charge, low and critical levels
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    charge = prop_number_integer_value((prop_number_t)pobj1);
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "warning-capacity")) )
                    low = prop_number_integer_value((prop_number_t)pobj1);
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "critical-capacity")) )
                    crit = prop_number_integer_value((prop_number_t)pobj1);
            }
            else if ( std::string(name, 0, 8) == "charging" ) {
                // charging or not?
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    if ( prop_number_integer_value((prop_number_t)pobj1) )
                        state |= XOSVIEW_BATT_CHARGING;
            }
            else if (std::string(name, 0, 14) == "discharge rate") {
                // discharging or not?
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    if ( prop_number_integer_value((prop_number_t)pobj1) )
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
    std::string battery;
    while (batteries < 1024) {
        float val = -1.0;
        battery = "acpibat" + util::repr(batteries);
        std::string emptyStr;
        BSDGetSensor(battery, "amphour0", val, emptyStr); // full capacity
        if (val < 0) // no more batteries
            break;
        batteries++;
        total_capacity += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour1", val, emptyStr); // warning capacity
        total_low += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour2", val, emptyStr); // low capacity
        total_crit += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour3", val, emptyStr); // remaining
        total_charge += val;
        emptyStr = "";
        BSDGetSensor(battery, "raw0", val, emptyStr); // state
        if ((int)val == 1)
            state |= XOSVIEW_BATT_DISCHARGING;
        else if ((int)val == 2)
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
    if ( !(state & XOSVIEW_BATT_CHARGING) &&
      !(state & XOSVIEW_BATT_DISCHARGING) )
        state |= XOSVIEW_BATT_FULL;  // full when not charging nor discharging
    if (total_capacity < total_low)
        state |= XOSVIEW_BATT_LOW;
    if (total_capacity < total_crit)
        state |= XOSVIEW_BATT_CRITICAL;
#else // XOSVIEW_FREEBSD || XOSVIEW_DFBSD
    /* Adapted from acpiconf and apm. */
    int fd;
    if ( (fd = open(ACPIDEV, O_RDONLY)) == -1 ) {
        // No ACPI -> try APM
        if ( (fd = open(APMDEV, O_RDONLY)) == -1 )
            logFatal << "could not open " << ACPIDEV << " or " << APMDEV
                     << std::endl;

        struct apm_info aip;
        if ( ioctl(fd, APMIO_GETINFO, &aip) == -1 )
            logFatal << "failed to get APM battery info" << std::endl;
        if ( close(fd) == -1 )
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
        return;
    }
    // ACPI
    union acpi_battery_ioctl_arg battio;
    battio.unit = ACPI_BATTERY_ALL_UNITS;
    if ( ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1 )
        logFatal << "failed to get ACPI battery info" << std::endl;
    if ( close(fd) == -1 )
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
#endif
}
