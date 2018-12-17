// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "tls_statistics_metrics_wrapper.h"

namespace storage {

TlsStatisticsMetricsWrapper::TlsStatisticsMetricsWrapper(metrics::MetricSet* owner)
    : metrics::MetricSet("network", {}, "Network connection metrics", owner),
      insecure_client_connections_established("insecure_client_connections_established", {},
              "Number of insecure (plaintext) client connections established", this),
      insecure_server_connections_accepted("insecure_server_connections_accepted", {},
              "Number of insecure (plaintext) server connections accepted", this),
      tls_client_connections_established("tls_client_connections_established", {},
              "Number of secure mTLS client connections established", this),
      tls_server_connections_accepted("tls_server_connections_accepted", {},
              "Number of secure mTLS server connections accepted", this),
      tls_handshakes_failed("tls_handshakes_failed", {}, "Number of client or "
              "server connection attempts that failed during TLS handshaking", this),
      peer_authorization_failures("peer_authorization_failures", {},
              "Number of TLS connection attempts failed due to bad or missing "
              "peer certificate credentials", this),
      tls_connections_broken("tls_connections_broken", {}, "Number of TLS "
              "connections broken due to failures during frame encoding or decoding", this),
      failed_tls_config_reloads("failed_tls_config_reloads", {}, "Number of times "
              "background reloading of TLS config has failed", this),
      last_client_stats_snapshot(),
      last_server_stats_snapshot(),
      last_config_stats_snapshot()
{}

TlsStatisticsMetricsWrapper::~TlsStatisticsMetricsWrapper() = default;

void TlsStatisticsMetricsWrapper::update_metrics_with_snapshot_delta() {
    auto server_current = vespalib::net::tls::ConnectionStatistics::get(true).snapshot();
    auto client_current = vespalib::net::tls::ConnectionStatistics::get(false).snapshot();
    auto server_delta = server_current.subtract(last_server_stats_snapshot);
    auto client_delta = client_current.subtract(last_client_stats_snapshot);

    insecure_client_connections_established.set(client_delta.insecure_connections);
    insecure_server_connections_accepted.set(server_delta.insecure_connections);
    tls_client_connections_established.set(client_delta.tls_connections);
    tls_server_connections_accepted.set(server_delta.tls_connections);
    // We have underlying stats for both server and client here, but for the
    // moment we just aggregate them up into combined metrics. Can be trivially
    // split up into separate metrics later if deemed useful.
    tls_handshakes_failed.set(client_delta.failed_tls_handshakes +
                              server_delta.failed_tls_handshakes);
    peer_authorization_failures.set(client_delta.invalid_peer_credentials +
                                    server_delta.invalid_peer_credentials);
    tls_connections_broken.set(client_delta.broken_tls_connections +
                               server_delta.broken_tls_connections);

    auto config_current = vespalib::net::tls::ConfigStatistics::get().snapshot();
    auto config_delta = config_current.subtract(last_config_stats_snapshot);

    failed_tls_config_reloads.set(config_delta.failed_config_reloads);

    last_server_stats_snapshot = server_current;
    last_client_stats_snapshot = client_current;
    last_config_stats_snapshot = config_current;
}

}
