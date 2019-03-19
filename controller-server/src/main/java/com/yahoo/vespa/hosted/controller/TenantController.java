// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller;

import com.yahoo.config.provision.TenantName;
import com.yahoo.vespa.curator.Lock;
import com.yahoo.vespa.hosted.controller.concurrent.Once;
import com.yahoo.vespa.hosted.controller.permits.AccessControlManager;
import com.yahoo.vespa.hosted.controller.permits.TenantSpecification;
import com.yahoo.vespa.hosted.controller.persistence.CuratorDb;
import com.yahoo.vespa.hosted.controller.tenant.AthenzTenant;
import com.yahoo.vespa.hosted.controller.tenant.Tenant;
import com.yahoo.vespa.hosted.controller.tenant.UserTenant;

import java.security.Principal;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.function.Consumer;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;

/**
 * A singleton owned by the Controller which contains the methods and state for controlling tenants.
 *
 * @author bratseth
 * @author mpolden
 */
public class TenantController {

    private static final Logger log = Logger.getLogger(TenantController.class.getName());

    private final Controller controller;
    private final CuratorDb curator;
    private final AccessControlManager permits;

    public TenantController(Controller controller, CuratorDb curator, AccessControlManager permits) {
        this.controller = Objects.requireNonNull(controller, "controller must be non-null");
        this.curator = Objects.requireNonNull(curator, "curator must be non-null");
        this.permits = permits;

        // Update serialization format of all tenants
        Once.after(Duration.ofMinutes(1), () -> {
            Instant start = controller.clock().instant();
            int count = 0;
            for (TenantName name : curator.readTenantNames()) {
                lockIfPresent(name, LockedTenant.class, this::store);
                count++;
            }
            log.log(Level.INFO, String.format("Wrote %d tenants in %s", count,
                                              Duration.between(start, controller.clock().instant())));
        });
    }

    /** Returns a list of all known tenants sorted by name */
    public List<Tenant> asList() {
        return curator.readTenants().stream()
                      .sorted(Comparator.comparing(Tenant::name))
                      .collect(Collectors.toList());
    }

    /** Returns the lsit of tenants accessible to the given user. */
    public List<Tenant> asList(Principal user) {
        return permits.accessibleTenants(asList(), user);
    }

    /** Locks a tenant for modification and applies the given action. */
    public <T extends LockedTenant> void lockIfPresent(TenantName name, Class<T> token, Consumer<T> action) {
        try (Lock lock = lock(name)) {
            get(name).map(tenant -> LockedTenant.of(tenant, lock))
                     .map(token::cast)
                     .ifPresent(action);
        }
    }

    /** Lock a tenant for modification and apply action. Throws if the tenant does not exist */
    public <T extends LockedTenant> void lockOrThrow(TenantName name, Class<T> token, Consumer<T> action) {
        try (Lock lock = lock(name)) {
            action.accept(token.cast(LockedTenant.of(require(name), lock)));
        }
    }

    /** Returns the tenant with the given name, or throws. */
    public Tenant require(TenantName name) {
        return get(name).orElseThrow(() -> new IllegalArgumentException("No such tenant '" + name + "'."));
    }

    /** Replace and store any previous version of given tenant */
    public void store(LockedTenant tenant) {
        curator.writeTenant(tenant.get());
    }

    /** Create an user tenant with given username */
    public void createUser(UserTenant tenant) {
        try (Lock lock = lock(tenant.name())) {
            requireNonExistent(tenant.name());
            curator.writeTenant(tenant);
        }
    }

    /** Create a tenant, provided the given permit is valid. */
    public void create(TenantSpecification permit) {
        try (Lock lock = lock(permit.tenant())) {
            requireNonExistent(permit.tenant());
            curator.writeTenant(permits.createTenant(permit, asList(), Collections.emptyList()));
        }
    }

    /** Find tenant by name */
    public Optional<Tenant> get(TenantName name) {
        return curator.readTenant(name);
    }

    /** Find tenant by name */
    public Optional<Tenant> get(String name) {
        return get(TenantName.from(name));
    }

    /** Find Athenz tenant by name */
    public Optional<AthenzTenant> athenzTenant(TenantName name) {
        return curator.readTenant(name)
                      .filter(AthenzTenant.class::isInstance)
                      .map(AthenzTenant.class::cast);
    }

    /** Returns Athenz tenant with name or throws if no such tenant exists */
    public AthenzTenant requireAthenzTenant(TenantName name) {
        return athenzTenant(name).orElseThrow(() -> new IllegalArgumentException("Tenant '" + name + "' not found"));
    }

    /** Updates the tenant contained in the given permit with new data. */
    public void update(TenantSpecification permit) {
        try (Lock lock = lock(permit.tenant())) {
            Tenant tenant = require(permit.tenant());
            List<Tenant> otherTenants = new ArrayList<>(asList());
            otherTenants.remove(tenant);

            List<Application> applications = controller.applications().asList(permit.tenant());
            permits.deleteTenant(permit, tenant, applications);
            curator.writeTenant(permits.createTenant(permit, otherTenants, applications));
        }
    }

    /** Deletes the tenant in the given permit. */
    public void delete(TenantSpecification permit) {
        try (Lock lock = lock(permit.tenant())) {
            Tenant tenant = require(permit.tenant());
            if ( ! controller.applications().asList(tenant.name()).isEmpty())
                throw new IllegalArgumentException("Could not delete tenant '" + tenant.name().value()
                                                   + "': This tenant has active applications");

            curator.removeTenant(tenant.name());
            permits.deleteTenant(permit, tenant, controller.applications().asList(permit.tenant()));
        }
    }

    /** Deletes the given user tenant. */
    public void deleteUser(UserTenant tenant) {
        try (Lock lock = lock(tenant.name())) {
            curator.removeTenant(tenant.name());
        }
    }

    private void requireNonExistent(TenantName name) {
        if (get(name).isPresent() ||
            // Underscores are allowed in existing Athenz tenant names, but tenants with - and _ cannot co-exist. E.g.
            // my-tenant cannot be created if my_tenant exists.
            get(dashToUnderscore(name.value())).isPresent()) {
            throw new IllegalArgumentException("Tenant '" + name + "' already exists");
        }
    }

    /**
     * Returns a lock which provides exclusive rights to changing this tenant.
     * Any operation which stores a tenant need to first acquire this lock, then read, modify
     * and store the tenant, and finally release (close) the lock.
     */
    private Lock lock(TenantName tenant) {
        return curator.lock(tenant);
    }

    private static String dashToUnderscore(String s) {
        return s.replace('-', '_');
    }

}
