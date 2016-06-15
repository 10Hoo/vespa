// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

namespace ns_log {

class Component {
    Component();
    Component& operator = (const Component &);
    Component(const Component &);
    char *_name;
    char *_charLevels;
    unsigned int *_intLevels;

public:
    bool matches(const char *pattern);
    void modifyLevels(char *levels);
    void display();
    explicit Component(char *);
};

} // end namespace ns_log

