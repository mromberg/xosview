//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

//-------------------------------------------------------
// BASIC Data Meter Template.  See example.h for comments
//-------------------------------------------------------

#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "fieldmetergraph.h"

class TemplateMeter : public FieldMeterGraph {
public:
    TemplateMeter(XOSView *parent);
    virtual ~TemplateMeter(void);

    virtual std::string name( void ) const { return "TemplateMeter"; }
    virtual void checkevent( void );
    virtual void checkResources( void );

private:
};

//-----------CUT-CUT-CUT---------------------------------
//-------------------------------------------------------
// For .cc file
//-------------------------------------------------------
#if 0  // REMOVE

TemplateMeter::TemplateMeter(XOSView *parent)
    : FieldMeterGraph(parent, 2, "TMPL", "VALUE/USED", true, true, true) {
}

TemplateMeter::~TemplateMeter(void) {
}

void TemplateMeter::checkResources( void ) {

    FieldMeterGraph::checkResources();

    // change "template" to real meter name
    setfieldcolor(0, parent_->getResource("templateForeGround"));
    setfieldcolor(1, parent_->getResource("templateBackground"));

    priority_ = util::stoi(parent_->getResource("templatePriority"));
    dodecay_ = parent_->isResourceTrue("templateDecay");
    useGraph_ = parent_->isResourceTrue("templateGraph");
    setUsedFormat(parent_->getResource("templateUsedFormat"));
    decayUsed(parent_->isResourceTrue("templateUsedDecay"));
}

void TemplateMeter::checkevent( void ) {
    total_ = 1.0;
    fields_[0] = 0.4;
    fields_[1] = 0.6;
    setUsed(0.4, 1.0);
    drawfields(parent_->g());
}

#endif // REMOVE
//-----------CUT-CUT-CUT---------------------------------

#endif // end TEMPLATE_H
