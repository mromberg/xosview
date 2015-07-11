//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

//-------------------------------------------------------
// Example data collection meter
//-------------------------------------------------------

// header data guards
#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "fieldmetergraph.h"  // our parent drawing class

class ExampleMeter : public FieldMeterGraph {
public:
    ExampleMeter(XOSView *parent);
    virtual ~ExampleMeter(void);

    // virtual methods called by our parents
    // when we should take action (more below)
    virtual std::string name( void ) const;
    virtual void checkevent( void );
    virtual void checkResources( void );

private:
    // Everything specific to our meter goes here
    // For this example I will just display something
    // silly.

    // Note:  Our parent meter has a fields_ vector
    // and total_ of type float
    // that we use to have it draw the
    // display for us.  This data type is perfectly
    // fine for this purpose.  But may be unsuitable
    // for use as a storage for our data values.
    // This is fine.  Define and use any suitable type
    // here

    unsigned long long _warpCoreTemp;   // current value
    unsigned long long _testMaximum;    // test max temp
    unsigned long long _designMaximum;  // design max temp

    void readWarpCoreTemp(void);        // read the current temp
};

//-------------------------------------------------------
// Normally this would be in a .cc file but it is here
// to keep this example in one file
//-------------------------------------------------------


// Constructor.  We initialize our parent and all of our
// private data.  We can not read any resources or cause
// any drawing from here because the window and
// assiciated graphics do not yet exist when a meter
// is constructed.
inline ExampleMeter::ExampleMeter(XOSView *parent)
    : FieldMeterGraph(parent, 2, "WCOR", "TEMP/USED", true, true, true),
      _warpCoreTemp(0), _testMaximum(0), _designMaximum(0) {

    // Generally leave the ctor empty.  But is is ok to call
    // a data gathering method as long as it does not depend
    // on any resource values or cause anything to be drawn.
    // Don't call any base class methods from here.

    // We know this is safe 'cause it is ours and we
    // do not do any of the above.  it will set our initial
    // values.
    readWarpCoreTemp();
}

// Usual dtor stuff.  If you dynamically allocated
// things destroy them here.  Etc...
inline ExampleMeter::~ExampleMeter(void) {
}

// Called when the rest of the system is ready
// for you to check any resources.  It is now
// safe to use the parents resource methods
// and allocate colors.  But you should not cause
// a draw to happen yet.
inline void ExampleMeter::checkResources( void ) {

    // First call our parent to have it check any resources
    // it needs.
    FieldMeterGraph::checkResources();

    // Normally you would add your new resource
    // items to Xdefaults.in and then be able to
    // call parent_->getResource() and know that
    // it would always find a value.  Since the
    // resources here are not in the Xdefaults file we
    // will use the longer form getResourceOrUseDefault()
    // But normally you can just use the shoter form.

    // Set some display properties in the base class
    // based on our specific resources.  Eventually
    // this will all be done like setUsedFormat().
    // For now some you set directly.  A few I've hardcoded
    // because isResourceTrue() does not take a default.
    // This is not a complete list of the options.  Look
    // at the parent meter class for more details.

    // First our own private values.
    _testMaximum = util::stoi(parent_->getResourceOrUseDefault(
          "exampleTestMax", "500"));
    _designMaximum = util::stoi(parent_->getResourceOrUseDefault(
          "exampleDesignMax", "550"));

    // And our parent's to control display
    priority_ = util::stoi(parent_->getResourceOrUseDefault(
          "examplePriority", "10"));
    setUsedFormat(parent_->getResourceOrUseDefault(
          "exampleUsedFormat", "percent"));

    // set the color with a string (requires a lookup)
    setfieldcolor(0, "green");

    // Set the color with a pixel value we may
    // save to avoid repeated lookups.  This is
    // just an example since we are not saving it
    // to show the use of the graphics object
    // from here.  Eventually this object will be passed in

    unsigned long color = parent_->g().allocColor("blue");
    setfieldcolor(1, color);
}

// Called when we have scheduled our data collection
// time.  The xosview clock ticks 10 times /sec and
// our priority is 10.  So, we should be called around
// once a second to update our data for our parent to
// display.
inline void ExampleMeter::checkevent( void ) {

    // update our values
    readWarpCoreTemp();

    // Now set values in the fields_.  Note that there
    // is no reason you have to store the actual measured
    // value in these fields.  They are just used to draw
    // So attempting to use them to store things like
    // the total number of bytes sent on a network interface
    // is going to run into trouble.  Do a conversion here.

    // For example we will use a scale of 0 - 1.0 where
    // 1.0 maps to the _testMaximum.
    float percentVal = static_cast<float>(_warpCoreTemp)
        / static_cast<float>(_testMaximum);
    total_ = 1.0;
    fields_[0] = percentVal;
    if (fields_[0] > 1.0) // peak the meter
        fields_[0] = 1.0;
    fields_[1] = 1.0 - fields_[0];

    // Change the field color to show our level of concern
    if (fields_[0] < .75)
        setfieldcolor(0, "green");
    else if (fields_[0] < 1.0)
        setfieldcolor(0, "yellow");
    else
        setfieldcolor(0, "red");

    // we can have the used label report the actual value even
    // if we peak the meter.
    setUsed(percentVal, total_);

    // Call the parent to draw.  Eventually this will not be needed.
    drawfields(parent_->g());
}

// What is your name?
inline std::string ExampleMeter::name( void ) const {
    return "ExampleMeter";
}

inline void ExampleMeter::readWarpCoreTemp(void) {
    static size_t count = 0;

    count++;

    _warpCoreTemp = count * 10;

    if (_warpCoreTemp > _designMaximum + 30) // Q hits reset switch
        count = 0;
}


//---------------------------------------------------------
// This meter is compiled into the linux and bsd ports
// so you can see how to add a new meter to the MeterMaker
// class.  Look in bsd/MeterMaker.cc or linux/MeterMaker.cc
// for more.
//
// To turn on the above meter code just run:
//
//      xosview -o "example: True"
//
//  A real meter would of course put it's .h and .cc files
// into the config/Makefile.os.in for their os so the code
// is only built for the os that supports it.
//---------------------------------------------------------

#endif
