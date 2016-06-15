// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * @file exceptions.h
 * @ingroup util
 *
 * @brief Various common exception classes.
 *
 * @author H�kon Humberset
 * @date 2004-03-31
 * @version $Id$
 */

#pragma once

#include <vespa/vespalib/util/exception.h>

namespace vespalib {

#ifdef IAM_DOXYGEN
/**
 * @class vespalib::UnsupportedOperationException
 * @brief Exception telling that the requested operation is not supported.
 **/
/**
 * @class vespalib::IllegalArgumentException
 * @brief Exception telling that illegal arguments was passed to a function.
 **/
/**
 * @class vespalib::IllegalStateException
 * @brief Exception telling that the object has an illegal state.
 **/
/**
 * @class vespalib::OverflowException
 * @brief XXX - Exception telling that some sort of overflow happened? unused
 **/
/**
 * @class vespalib::UnderflowException
 * @brief XXX - Exception telling that some sort of underflow happened? unused
 **/
/**
 * @class vespalib::FatalException
 * @brief Something went seriously wrong and the application should terminate
 **/

#else

VESPA_DEFINE_EXCEPTION(UnsupportedOperationException, Exception);
VESPA_DEFINE_EXCEPTION(IllegalArgumentException, Exception);
VESPA_DEFINE_EXCEPTION(IllegalStateException, Exception);
VESPA_DEFINE_EXCEPTION(OverflowException, Exception);
VESPA_DEFINE_EXCEPTION(UnderflowException, Exception);
VESPA_DEFINE_EXCEPTION(TimeoutException, Exception);
VESPA_DEFINE_EXCEPTION(FatalException, Exception);
VESPA_DEFINE_EXCEPTION(NetworkSetupFailureException, IllegalStateException);

#endif

//-----------------------------------------------------------------------------

/**
 * @brief Exception indicating the failure to listen for connections
 * on a socket.
 **/
class PortListenException : public Exception
{
private:
    int _port;
    vespalib::string _protocol;

    vespalib::string make_message(int port, const vespalib::stringref &protocol,
                                  const vespalib::stringref &msg);

public:
    PortListenException(int port, const vespalib::stringref &protocol,
                        const vespalib::stringref &msg = "",
                        const vespalib::stringref &location = "", int skipStack = 0);
    PortListenException(int port, const vespalib::stringref &protocol,
                        const Exception &cause,
                        const vespalib::stringref &msg = "",
                        const vespalib::stringref &location = "", int skipStack = 0);
    VESPA_DEFINE_EXCEPTION_SPINE(PortListenException);
    int get_port() const { return _port; }
    const vespalib::string &get_protocol() const { return _protocol; }
};

//-----------------------------------------------------------------------------

/**
 * @brief Exception signaling that some sort of I/O error happened.
 **/
class IoException : public Exception {
public:
    enum Type { UNSPECIFIED,
                ILLEGAL_PATH, NO_PERMISSION, DISK_PROBLEM,
                INTERNAL_FAILURE, NO_SPACE, NOT_FOUND, CORRUPT_DATA,
                TOO_MANY_OPEN_FILES, DIRECTORY_HAVE_CONTENT, FILE_FULL,
                ALREADY_EXISTS };

    IoException(const stringref & msg, Type type, const stringref & location,
                int skipStack = 0);
    IoException(const stringref & msg, Type type, const Exception& cause,
                const stringref & location, int skipStack = 0);

    VESPA_DEFINE_EXCEPTION_SPINE(IoException);

    static string createMessage(const stringref  & msg, Type type);

    Type getType() const { return _type; }

    /** Use this function as a way to map error from errno to a Type. */
    static Type getErrorType(int error);

private:
    Type _type;
};

}

