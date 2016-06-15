// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "ihopdirective.h"

namespace mbus {

/**
 * This class represents a policy directive within a {@link Hop}'s selector. This means to create the named protocol
 * using the given parameter string, and the running that protocol within the context of this directive.
 *
 * @author <a href="mailto:simon@yahoo-inc.com">Simon Thoresen</a>
 * @version $Id$
 */
class PolicyDirective : public IHopDirective {
private:
    string _name;
    string _param;

public:

    /**
     * Constructs a new policy selector item.
     *
     * @param name  The name of the policy to invoke.
     * @param param The parameter to pass to the name constructor.
     */
    PolicyDirective(const vespalib::stringref &name, const vespalib::stringref &param);

    /**
     * Returns the name of the policy that this item is to invoke.
     *
     * @return The name name.
     */
    const string &getName() const { return _name; }

    /**
     * Returns the parameter string for this policy directive.
     *
     * @return The parameter.
     */
    const string &getParam() const { return _param; }

    virtual Type getType() const { return TYPE_POLICY; }
    virtual bool matches(const IHopDirective &) const { return true; }
    virtual string toString() const;
    virtual string toDebugString() const;
};

} // mbus

