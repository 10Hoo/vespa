// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.logserver.net;

import com.yahoo.log.LogLevel;
import com.yahoo.logserver.LogDispatcher;
import com.yahoo.log.LogMessage;
import com.yahoo.log.InvalidLogFormatException;

import com.yahoo.io.Connection;
import com.yahoo.io.Listener;
import com.yahoo.io.ReadLine;

import com.yahoo.logserver.net.control.Levels;

import java.io.IOException;
import java.util.logging.Logger;
import java.util.logging.Level;
import java.util.LinkedList;
import java.util.Set;
import java.util.HashSet;

import java.nio.charset.Charset;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.nio.channels.SelectionKey;

/**
 * TODO
 * <UL>
 *  <LI> send invalid log messages to somewhere so they can be
 *       analyzed and errors can be corrected.
 * </UL>
 *
 * @author  <a href="mailto:borud@yahoo-inc.com">Bjorn Borud</a>
 */

public class LogConnection implements Connection {
    private static final Logger log = Logger.getLogger(LogConnection.class.getName());

    public static final int READBUFFER_SIZE = (32 * 1024);

    static private final Charset charset = Charset.forName("utf-8");

    // the set of active connections
    private static final Set<LogConnection> activeConnections = new HashSet<>();

    private final SocketChannel socket;
    private final Listener listener;
    private final LogDispatcher dispatcher;

    private final ByteBuffer readBuffer = ByteBuffer.allocateDirect(READBUFFER_SIZE);

    private final LinkedList<ByteBuffer> writeBufferList = new LinkedList<>();
    private ByteBuffer writeBuffer;

    // counters
    private long totalBytesRead = 0;
    private long totalBytesWritten = 0;

    // default log levels for logd
    final Levels defaultLevels;

    public LogConnection (SocketChannel socket,
                          Listener listener,
                          LogDispatcher dispatcher,
                          Levels defaultLevels) {
        this.socket = socket;
        this.listener = listener;
        this.dispatcher = dispatcher;
        this.defaultLevels = defaultLevels;

        addToActiveSet(this);

        // send the "setdefaultstate" command to logd. no better
        // place to put it for now.
        sendDefaultState();
    }

    /**
     * Send the default state to the
     */
    public void sendDefaultState () {
        if (defaultLevels == null) {
            return;
        }

        try {
            enqueue(charset.encode("setdefaultstate "
                                   + defaultLevels.toString()
                                   + "\n"));
            enqueue(charset.encode("setallstates "
                    + defaultLevels.toString()
                    + "\n"));
        }
        catch (IOException e) {
            log.log(LogLevel.WARNING, "Unable to send default state", e);
        }
    }

    /**
     * Return a shallow copy of the set of active connections.
     *
     */
    public static Set<LogConnection> getActiveConnections () {
        synchronized(activeConnections) {
        	return new HashSet<>(activeConnections);
        }
    }

    /**
     * @return Return total number of bytes written to connection
     */
    public long getTotalBytesWritten () {
        return totalBytesWritten;
    }

    /**
     * @return Return total number of bytes read from connection
     */
    public long getTotalBytesRead () {
        return totalBytesRead;
    }

    /**
     * Internal method for adding connection to the set
     * of active connections.
     *
     * @param connection The connection to be added
     */
    private static void addToActiveSet (LogConnection connection) {
        synchronized(activeConnections) {
            activeConnections.add(connection);
        }
    }

    /**
     * Internal method to remove connection from the set of
     * active connections.
     *
     * @param connection The connection to remove
     * @throws IllegalStateException if the connection does not
     *                               exist in the set
     *
     */
    private static void removeFromActiveSet (LogConnection connection) {
        synchronized(activeConnections) {
            activeConnections.remove(connection);
        }
    }

    /**
     *
     */
    public synchronized void enqueue (ByteBuffer buffer) throws IOException {
        if (writeBuffer == null) {
            writeBuffer = buffer;
        } else {
            writeBufferList.addLast(buffer);
            listener.modifyInterestOps(this, SelectionKey.OP_WRITE, true);
        }
        write();
    }

    public void connect () throws IOException {
        throw new RuntimeException("connect() is not supposed to be called");
    }

    public synchronized void write () throws IOException {
        int bytesWritten;
        do {
            // if writeBufferList is not set we need to fetch the next buffer
            if (writeBuffer == null) {

                // if the list is empty, signal the selector we do not need
                // to do any writing for a while yet and bail
                if (writeBufferList.isEmpty()) {
                    listener.modifyInterestOpsBatch(this,
                                                    SelectionKey.OP_WRITE,
                                                    false);
                    return;
                }
                writeBuffer = writeBufferList.removeFirst();
            }

            // invariants: we have a writeBuffer

            bytesWritten = socket.write(writeBuffer);
            totalBytesWritten += bytesWritten;

            // buffer drained so we forget it and see what happens when we
            // go around.  if indeed we go around
            if ((writeBuffer != null) && (!writeBuffer.hasRemaining())) {
                writeBuffer = null;
            }
        } while (bytesWritten > 0);
    }

    public void read() throws IOException {
        if (! readBuffer.hasRemaining()) {

            try {
                readBuffer.putChar(readBuffer.capacity() - 2, '\n');
                readBuffer.flip();
                String s = ReadLine.readLine(readBuffer);
                if (s == null) {
                    return;
                }
                int count = 200;
                log.log(LogLevel.WARNING, "Log message too long. Message from "
                        + socket.socket().getInetAddress() +  " exceeds "
                        + readBuffer.capacity()
                        + ". Skipping buffer (might be part of same long message). Printing first " + count + " characters of line: " +
                s.substring(0, count));

                LogMessage msg = LogMessage.parseNativeFormat(s);
                dispatcher.handle(msg);
            }
            catch (InvalidLogFormatException e) {
                log.log(LogLevel.DEBUG, "Invalid log message", e);
            }
            finally {
                readBuffer.clear();
            }
            return;
        }

        int ret = socket.read(readBuffer);
        if (ret == -1) {
            close();
            return;
        }

        if (ret == 0) {
            if (log.isLoggable(Level.FINE)) {
                log.log(LogLevel.DEBUG, "zero byte read occurred");
             }
        }

        // update global counter
        totalBytesRead += ret;

        readBuffer.flip();

        String s;
        while ((s = ReadLine.readLine(readBuffer)) != null) {
            try {
                LogMessage msg = LogMessage.parseNativeFormat(s);
                dispatcher.handle(msg);
            }
            catch (InvalidLogFormatException e) {
                log.log(LogLevel.DEBUG, "Invalid log message", e);
            }
        }
    }

    public void close() throws IOException {
        if (log.isLoggable(Level.FINE)) {
            log.log(LogLevel.INFO, this + ": closing");
        }
        socket.close();
        removeFromActiveSet(this);
    }

    public int selectOps() {
        return SelectionKey.OP_READ;
    }

    public SocketChannel socketChannel() {
        return socket;
    }

}
