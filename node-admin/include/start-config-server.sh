#!/bin/bash
# Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

# BEGIN environment bootstrap section
# Do not edit between here and END as this section should stay identical in all scripts

findpath () {
    myname=${0}
    mypath=${myname%/*}
    myname=${myname##*/}
    if [ "$mypath" ] && [ -d "$mypath" ]; then
        return
    fi
    mypath=$(pwd)
    if [ -f "${mypath}/${myname}" ]; then
        return
    fi
    echo "FATAL: Could not figure out the path where $myname lives from $0"
    exit 1
}

COMMON_ENV=libexec/vespa/common-env.sh

source_common_env () {
    if [ "$VESPA_HOME" ] && [ -d "$VESPA_HOME" ]; then
        # ensure it ends with "/" :
        VESPA_HOME=${VESPA_HOME%/}/
        export VESPA_HOME
        common_env=$VESPA_HOME/$COMMON_ENV
        if [ -f "$common_env" ]; then
            . $common_env
            return
        fi
    fi
    return 1
}

findroot () {
    source_common_env && return
    if [ "$VESPA_HOME" ]; then
        echo "FATAL: bad VESPA_HOME value '$VESPA_HOME'"
        exit 1
    fi
    if [ "$ROOT" ] && [ -d "$ROOT" ]; then
        VESPA_HOME="$ROOT"
        source_common_env && return
    fi
    findpath
    while [ "$mypath" ]; do
        VESPA_HOME=${mypath}
        source_common_env && return
        mypath=${mypath%/*}
    done
    echo "FATAL: missing VESPA_HOME environment variable"
    echo "Could not locate $COMMON_ENV anywhere"
    exit 1
}

findroot

# END environment bootstrap section

export LC_ALL=C

function WaitUntilHostIsReachable {
    # Address may be IP or hostname.
    local address="$1"

    echo -n "Will wait until $address is reachable... "
    while ! ping -q -c 1 -W 3 "$address" &>/dev/null
    do
        echo "not done (will retry)"
        sleep 1
    done
    echo "Done"
}

function VerifyRequiredEnvironmentVariablesAreSet {
    if [ -z "$HOSTED_VESPA_REGION" ]
    then
        Fail "Environment variable HOSTED_VESPA_REGION is not set"
    fi
    if [ -z "$CONFIG_SERVER_HOSTNAME" ]
    then
        Fail "Environment variable CONFIG_SERVER_HOSTNAME is not set"
    fi
    if [ -z "$HOST_BRIDGE_IP" ]
    then
        Fail "Environment variable HOST_BRIDGE_IP is not set"
    fi

    case "$HOSTED_VESPA_ENVIRONMENT" in
        prod|test|dev|staging|perf) : ;;
        *) Fail "The HOSTED_VESPA_ENVIRONMENT environment variable must be one of prod, test, dev, staging, or perf" ;;
    esac
}

function InternalMain {
    VerifyRequiredEnvironmentVariablesAreSet

    mkdir -p $VESPA_HOME/logs
    chmod 1777 $VESPA_HOME/logs
    mkdir -p $VESPA_HOME/logs/jdisc_core

    rm -rf $VESPA_HOME/var/vespa/bundlecache/standalone

    yinst set \
          cloudconfig_server.multitenant=true \
          cloudconfig_server.region="$HOSTED_VESPA_REGION" \
          cloudconfig_server.autostart=on \
          cloudconfig_server.default_flavor=docker \
          cloudconfig_server.environment="$HOSTED_VESPA_ENVIRONMENT" \
          cloudconfig_server.hosted_vespa=true \
          services.addr_configserver="$CONFIG_SERVER_HOSTNAME"

    # Can also set jvmargs if necessary:
    # set cloudconfig_server.jvmargs=-Dvespa.freezedetector.disable=true -XX:NewRatio=1 -verbose:gc -XX:+PrintGCDateStamps -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=5005 -Xms6g -Xmx6g

    # The network is set up asynchronously and from outside of this
    # container. Wait until it's done.
    WaitUntilHostIsReachable "$HOST_BRIDGE_IP"

    yinst start cloudconfig_server

    touch $VESPA_HOME/logs/jdisc_core/jdisc_core.log
    $VESPA_HOME/bin/logfmt -N -f $VESPA_HOME/logs/jdisc_core/jdisc_core.log
}

function Main {
    # Prefix each line to stdout/stderr with a timestamp to make it easier to
    # understand the progress.
    InternalMain |& while read -r
    do
        printf "%s %s\n" "$(date +%FT%T)" "$REPLY"
    done
}

Main "$@"
