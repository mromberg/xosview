//
//  Copyright (c) 2008, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  Read coretemp reading with sysctl and display actual temperature.
//  If actual >= high, actual temp changes color to indicate alarm.
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//

#include "coretemp.h"
#include "kernel.h"

#include <numeric>



CoreTemp::CoreTemp( const std::string &label, const std::string &caption,
  int cpu)
    : FieldMeter(3, label, caption),
      _cpu(cpu), _cpuCount(countCpus()),
      _high(0), _temps(_cpuCount, 0),
      _actColor(0), _highColor(0) {

    setMetric(true);
}


void CoreTemp::checkResources(const ResDB &rdb) {
    FieldMeter::checkResources(rdb);

    _actColor = rdb.getColor("coretempActColor");
    _highColor = rdb.getColor("coretempHighColor");

    setfieldcolor(0, _actColor);
    setfieldcolor(1, rdb.getColor("coretempIdleColor"));
    setfieldcolor(2, _highColor);

    const std::string highest = rdb.getResourceOrUseDefault(
        "coretempHighest", "100");
    _total = std::stoi(highest);
    const std::string high = rdb.getResourceOrUseDefault("coretempHigh", "");

    // Get tjMax here and use as total.
    std::vector<float> tjmax(_cpuCount, 0.0);
    BSDGetCPUTemperature(_temps, tjmax);
    float total = -300.0;
    for (const auto &tjm : tjmax) {
        if (tjm > total)
            total = tjm;
    }

    if (total > 0.0)
        _total = total;

    std::string lgnd;
    if (!high.size()) {
        _high = _total;
        lgnd = "ACT(uC)/HIGH/" + std::to_string(static_cast<int>(_total));
    }
    else {
        _high = std::stoi(high);
        lgnd = "ACT(uC)/" + std::to_string(static_cast<int>(_high))
            + "/" + std::to_string(static_cast<int>(_total));
    }
    legend(lgnd);
}


unsigned int CoreTemp::countCpus(void) {
    return BSDGetCPUTemperature();
}


void CoreTemp::checkevent(void) {
    getcoretemp();

    setUsed(_fields[0], _total);
    if (_fields[0] < 0)
        _fields[0] = 0;
    _fields[1] = _high - _fields[0];
    _fields[2] = _total - _fields[1] - _fields[0];
    if (_fields[0] > _total)
        _fields[0] = _total;
    if (_fields[2] < 0)
        _fields[2] = 0;

    if (_fields[1] < 0) { // alarm: T > high
        _fields[1] = 0;
        if (fieldcolor(0) != _highColor) {
            setfieldcolor(0, _highColor);
        }
    }
    else {
        if (fieldcolor(0) != _actColor) {
            setfieldcolor(0, _actColor);
        }
    }
}


void CoreTemp::getcoretemp(void) {
    BSDGetCPUTemperature(_temps);

    _fields[0] = 0.0;
    if (_cpu >= 0 && static_cast<size_t>(_cpu) < _cpuCount) { // one core
        _fields[0] = _temps[_cpu];
    }
    else if (_cpu == -1) {  // average
        const float sum = std::accumulate(_temps.cbegin(), _temps.cend(), 0.0);
        _fields[0] = _temps.empty() ? 0.0
            : sum / static_cast<float>(_temps.size());
    }
    else if (_cpu == -2) {  // maximum
        const auto it = std::max_element(_temps.cbegin(), _temps.cend());
        _fields[0] = (it == _temps.cend()) ? 0 : *it;
    }
    else {    // should not happen
        logFatal << "Unknown CPU core number in coretemp." << std::endl;
    }
}
