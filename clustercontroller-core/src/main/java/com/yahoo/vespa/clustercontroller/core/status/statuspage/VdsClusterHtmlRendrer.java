// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.clustercontroller.core.status.statuspage;

import com.yahoo.vdslib.state.ClusterState;
import com.yahoo.vdslib.state.NodeState;
import com.yahoo.vdslib.state.NodeType;
import com.yahoo.vdslib.state.State;
import com.yahoo.vespa.clustercontroller.core.EventLog;
import com.yahoo.vespa.clustercontroller.core.NodeInfo;
import com.yahoo.vespa.clustercontroller.core.RealTimer;
import com.yahoo.vespa.clustercontroller.core.Timer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;
import java.util.TreeMap;


/**
 * Renders webpage with status regarding cluster.
 */
public class VdsClusterHtmlRendrer {

    private static final TimeZone utcTimeZone = TimeZone.getTimeZone("UTC");

    public static class Table {
        private final HtmlTable table = new HtmlTable();
        private final HtmlTable.CellProperties headerProperties;
        private final StringBuilder contentBuilder = new StringBuilder();
        private final static String TAG_NOT_SET = "not set";

        Table(final String clusterName, final int slobrokGenerationCount) {
            table.getTableProperties().align(HtmlTable.Orientation.RIGHT).setBackgroundColor(0xc0ffc0);
            table.getColProperties(0).align(HtmlTable.Orientation.CENTER).setBackgroundColor(0xffffff);
            table.getColProperties(1).align(HtmlTable.Orientation.LEFT);
            table.getColProperties(2).align(HtmlTable.Orientation.LEFT);
            table.getColProperties(3).align(HtmlTable.Orientation.LEFT);
            table.getColProperties(7).align(HtmlTable.Orientation.LEFT);
            table.getColProperties(12).align(HtmlTable.Orientation.LEFT);
            for (int i = 4; i < 13; ++i) table.getColProperties(i).allowLineBreaks(false);
            headerProperties = new HtmlTable.CellProperties()
                    .setBackgroundColor(0xffffff)
                    .align(HtmlTable.Orientation.CENTER);
            contentBuilder.append("<h2>State of content cluster '")
                    .append(clusterName)
                    .append("'.</h2>\n")
                    .append("<p>Based on information retrieved from slobrok at generation ")
                    .append(slobrokGenerationCount).append(".</p>\n");
        }

        public void addTable(final StringBuilder destination, final long stableStateTimePeriode) {
            destination.append(contentBuilder);

            destination.append(table.toString())
                    .append("<p>")
                    .append("<p>");
            addFooter(destination, stableStateTimePeriode);
        }

        public void renderNodes(
                final TreeMap<Integer, NodeInfo> storageNodeInfos,
                final TreeMap<Integer, NodeInfo> distributorNodeInfos,
                final Timer timer,
                final ClusterState state,
                final int maxPrematureCrashes,
                final EventLog eventLog,
                final String pathPrefix,
                final String name) {
            final String dominantVtag = findDominantVtag(
                    storageNodeInfos, distributorNodeInfos);

            renderNodesOneType(storageNodeInfos,
                    NodeType.STORAGE,
                    timer,
                    state,
                    maxPrematureCrashes,
                    eventLog,
                    pathPrefix,
                    dominantVtag,
                    name);
            renderNodesOneType(distributorNodeInfos,
                    NodeType.DISTRIBUTOR,
                    timer,
                    state,
                    maxPrematureCrashes,
                    eventLog,
                    pathPrefix,
                    dominantVtag,
                    name);
        }

        private String findDominantVtag(
                final Map<Integer, NodeInfo> storageNodeInfos,
                final Map<Integer, NodeInfo> distributorNodeInfos) {
            final List<NodeInfo> nodeInfos = new ArrayList<>();
            nodeInfos.addAll(storageNodeInfos.values());
            nodeInfos.addAll(distributorNodeInfos.values());

            final Map<String, Integer> versionTagToCount = new HashMap<>();
            int maxCount = -1;
            String dominantVtag = null;
            for (NodeInfo nodeInfo : nodeInfos) {
                final String buildTag = nodeInfo.getVtag();
                Integer count = versionTagToCount.get(buildTag);
                count = count == null ? 1 : count + 1;
                versionTagToCount.put(buildTag, count);
                if (count > maxCount) {
                    maxCount = count;
                    dominantVtag = buildTag;
                }
            }
            return dominantVtag == null ? TAG_NOT_SET : dominantVtag;
        }
        private void addTableHeader(final String name, final NodeType nodeType) {
            table.addRow(new HtmlTable.Row().addCell(
                    new HtmlTable.Cell("Group " + name)
                            .addProperties(new HtmlTable.CellProperties()
                                    .setColSpan(0)
                                    .setBackgroundColor(0xccccff)
                                    .align(HtmlTable.Orientation.LEFT))));
            table.addRow(new HtmlTable.Row()
                    .setHeaderRow()
                    .addProperties(headerProperties)
                    .addProperties(new HtmlTable.CellProperties().setRowSpan(2))
                    .addCell(new HtmlTable.Cell(nodeType == NodeType.DISTRIBUTOR ? "Distributor" : "Storage"))
                    .addCell(new HtmlTable.Cell("Node states")
                            .addProperties(new HtmlTable.CellProperties().setColSpan(3).setRowSpan(1)))
                    .addCell(new HtmlTable.Cell("Build"))
                    .addCell(new HtmlTable.Cell("FC<sup>1)</sup>"))
                    .addCell(new HtmlTable.Cell("OCT<sup>2)</sup>"))
                    .addCell(new HtmlTable.Cell("SPT<sup>3)</sup>"))
                    .addCell(new HtmlTable.Cell("SSV<sup>4)</sup>"))
                    .addCell(new HtmlTable.Cell("PC<sup>5)</sup>"))
                    .addCell(new HtmlTable.Cell("ELW<sup>6)</sup>"))
                    .addCell(new HtmlTable.Cell("Start Time"))
                    .addCell(new HtmlTable.Cell("RPC Address")));
            table.addRow(new HtmlTable.Row().setHeaderRow().addProperties(headerProperties)
                    .addCell(new HtmlTable.Cell("Reported"))
                    .addCell(new HtmlTable.Cell("Wanted"))
                    .addCell(new HtmlTable.Cell("System")));
        }

        private void renderNodesOneType(
                final TreeMap<Integer, NodeInfo> nodeInfos,
                final NodeType nodeType,
                final Timer timer,
                final ClusterState state,
                final int maxPrematureCrashes,
                final EventLog eventLog,
                final String pathPrefix,
                final String dominantVtag,
                final String name) {
            final long currentTime = timer.getCurrentTimeInMillis();
            addTableHeader(name, nodeType);
            for (final NodeInfo nodeInfo : nodeInfos.values()) {
                HtmlTable.Row row = new HtmlTable.Row();
                HtmlTable.CellProperties warning = new HtmlTable.CellProperties().setBackgroundColor(0xffffc0);
                HtmlTable.CellProperties error = new HtmlTable.CellProperties().setBackgroundColor(0xffc0c0);
                HtmlTable.CellProperties centered = new HtmlTable.CellProperties().align(HtmlTable.Orientation.CENTER);

                // Add node index
                row.addCell(new HtmlTable.Cell("<a href=\"" + pathPrefix + "/node=" + nodeInfo.getNode()
                        + "\">" + nodeInfo.getNodeIndex() + "</a>"));

                // Add reported state
                NodeState reportedState = nodeInfo.getReportedState().clone().setStartTimestamp(0);
                row.addCell(new HtmlTable.Cell(HtmlTable.escape(reportedState.toString(true))));
                if (!nodeInfo.getReportedState().getState().equals(State.UP)) {
                    row.getLastCell().addProperties(warning);
                }

                // Add wanted state
                if (nodeInfo.getWantedState() == null || nodeInfo.getWantedState().getState().equals(State.UP)) {
                    row.addCell(new HtmlTable.Cell("-").addProperties(centered));
                } else {
                    row.addCell(new HtmlTable.Cell(HtmlTable.escape(nodeInfo.getWantedState().toString(true))));
                    if (nodeInfo.getWantedState().toString(true).indexOf("Disabled by fleet controller") != -1) {
                        row.getLastCell().addProperties(error);
                    } else {
                        row.getLastCell().addProperties(warning);
                    }
                }

                // Add current state
                NodeState ns = state.getNodeState(nodeInfo.getNode()).clone().setDescription("").setMinUsedBits(16);
                if (state.getClusterState().oneOf("uir")) {
                    row.addCell(new HtmlTable.Cell(HtmlTable.escape(ns.toString(true))));
                    if (ns.getState().equals(State.DOWN)) {
                        row.getLastCell().addProperties(error);
                    } else if (ns.getState().oneOf("mi")) {
                        row.getLastCell().addProperties(warning);
                    }
                } else {
                    row.addCell(new HtmlTable.Cell("Cluster " +
                            state.getClusterState().name().toLowerCase()).addProperties(error));
                }

                // Add build tag version.
                final String buildTagText =
                        nodeInfo.getVtag() != null
                                ? nodeInfo.getVtag()
                                : TAG_NOT_SET;
                row.addCell(new HtmlTable.Cell(buildTagText));
                if (! dominantVtag.equals(nodeInfo.getVtag())) {
                    row.getLastCell().addProperties(warning);
                }

                // Add failed connection attempt count
                row.addCell(new HtmlTable.Cell("" + nodeInfo.getConnectionAttemptCount()));
                long timeSinceContact = nodeInfo.getTimeOfFirstFailingConnectionAttempt() == 0
                        ? 0 : currentTime - nodeInfo.getTimeOfFirstFailingConnectionAttempt();
                if (timeSinceContact > 60 * 1000) {
                    row.getLastCell().addProperties(error);
                } else if (nodeInfo.getConnectionAttemptCount() > 0) {
                    row.getLastCell().addProperties(warning);
                }

                // Add time since first failing
                row.addCell(new HtmlTable.Cell((timeSinceContact / 1000) + " s"));
                if (timeSinceContact > 60 * 1000) {
                    row.getLastCell().addProperties(error);
                } else if (nodeInfo.getConnectionAttemptCount() > 0) {
                    row.getLastCell().addProperties(warning);
                }

                // State pending time
                if (nodeInfo.getLatestNodeStateRequestTime() == null) {
                    row.addCell(new HtmlTable.Cell("-").addProperties(centered));
                } else {
                    row.addCell(new HtmlTable.Cell(HtmlTable.escape(RealTimer.printDuration(
                            currentTime - nodeInfo.getLatestNodeStateRequestTime()))));
                }

                // System state version
                row.addCell(new HtmlTable.Cell("" + nodeInfo.getSystemStateVersionAcknowledged()));
                if (nodeInfo.getSystemStateVersionAcknowledged() < state.getVersion() - 2) {
                    row.getLastCell().addProperties(error);
                } else if (nodeInfo.getSystemStateVersionAcknowledged() < state.getVersion()) {
                    row.getLastCell().addProperties(warning);
                }

                // Premature crashes
                row.addCell(new HtmlTable.Cell("" + nodeInfo.getPrematureCrashCount()));
                if (nodeInfo.getPrematureCrashCount() >= maxPrematureCrashes) {
                    row.getLastCell().addProperties(error);
                } else if (nodeInfo.getPrematureCrashCount() > 0) {
                    row.getLastCell().addProperties(warning);
                }

                // Events last week
                int nodeEvents = eventLog.getNodeEventsSince(nodeInfo.getNode(),
                        currentTime - eventLog.getRecentTimePeriod());
                row.addCell(new HtmlTable.Cell("" + nodeEvents));
                if (nodeEvents > 20) {
                    row.getLastCell().addProperties(error);
                } else if (nodeEvents > 3) {
                    row.getLastCell().addProperties(warning);
                }

                // Start time
                if (nodeInfo.getStartTimestamp() == 0) {
                    row.addCell(new HtmlTable.Cell("-").addProperties(error).addProperties(centered));
                } else {
                    String startTime = RealTimer.printDateNoMilliSeconds(
                            1000 * nodeInfo.getStartTimestamp(), utcTimeZone);
                    row.addCell(new HtmlTable.Cell(HtmlTable.escape(startTime)));
                }

                // RPC address
                if (nodeInfo.getRpcAddress() == null) {
                    row.addCell(new HtmlTable.Cell("-").addProperties(error));
                } else {
                    row.addCell(new HtmlTable.Cell(HtmlTable.escape(nodeInfo.getRpcAddress())));
                    if (nodeInfo.isRpcAddressOutdated()) {
                        row.getLastCell().addProperties(warning);
                    }
                }
                table.addRow(row);
            }
        }
        private void addFooter(final StringBuilder contentBuilder, final long stableStateTimePeriode) {
            contentBuilder.append("<font size=\"-1\">\n")
                    .append("1) FC - Failed connections - We have tried to connect to the nodes this many times " +
                            "without being able to contact it.<br>\n")
                    .append("2) OCT - Out of contact time - Time in seconds we have failed to contact the node.<br>\n")
                    .append("3) SPT - State pending time - Time the current getNodeState request has been " +
                            "pending.<br>\n")
                    .append("4) SSV - System state version - The latest system state version the node has " +
                            "acknowledged.<br>\n")
                    .append("5) PC - Premature crashes - Number of times node has crashed since last time it had " +
                            "been stable in up or down state for more than "
                            + RealTimer.printDuration(stableStateTimePeriode) + ".<br>\n")
                    .append("6) ELW - Events last week - The number of events that has occured on this node the " +
                            "last week. (Or shorter period if a week haven't passed since restart or more than " +
                            "max events to keep in node event log have happened during last week.)<br>\n")
                    .append("</font>\n");
        }
    }

    public Table createNewClusterHtmlTable(final String clusterName, final int slobrokGenerationCount) {
        return new Table(clusterName, slobrokGenerationCount);
    }

}
