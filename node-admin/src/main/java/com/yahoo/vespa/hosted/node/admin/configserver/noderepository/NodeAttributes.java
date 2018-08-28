// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.configserver.noderepository;

import com.yahoo.vespa.hosted.dockerapi.DockerImage;

import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class NodeAttributes {

    private Optional<Long> restartGeneration = Optional.empty();
    private Optional<Long> rebootGeneration = Optional.empty();
    private Optional<DockerImage> dockerImage = Optional.empty();
    private Optional<String> vespaVersion = Optional.empty();
    private Optional<String> currentOsVersion = Optional.empty();
    private Optional<String> hardwareDivergence = Optional.empty();
    private Optional<String> hardwareFailureDescription = Optional.empty();
    private Optional<Boolean> wantToDeprovision = Optional.empty();

    public NodeAttributes() { }

    public NodeAttributes withRestartGeneration(Optional<Long> restartGeneration) {
        this.restartGeneration = restartGeneration;
        return this;
    }

    public NodeAttributes withRebootGeneration(long rebootGeneration) {
        this.rebootGeneration = Optional.of(rebootGeneration);
        return this;
    }

    public NodeAttributes withDockerImage(DockerImage dockerImage) {
        this.dockerImage = Optional.of(dockerImage);
        return this;
    }

    public NodeAttributes withVespaVersion(String vespaVersion) {
        this.vespaVersion = Optional.of(vespaVersion);
        return this;
    }

    public NodeAttributes withCurrentOsVersion(String currentOsVersion) {
        this.currentOsVersion = Optional.of(currentOsVersion);
        return this;
    }

    public NodeAttributes withHardwareDivergence(String hardwareDivergence) {
        this.hardwareDivergence = Optional.of(hardwareDivergence);
        return this;
    }

    public NodeAttributes withHardwareFailureDescription(String hardwareFailureDescription) {
        this.hardwareFailureDescription = Optional.of(hardwareFailureDescription);
        return this;
    }

    public NodeAttributes withWantToDeprovision(boolean wantToDeprovision) {
        this.wantToDeprovision = Optional.of(wantToDeprovision);
        return this;
    }


    public Optional<Long> getRestartGeneration() {
        return restartGeneration;
    }

    public Optional<Long> getRebootGeneration() {
        return rebootGeneration;
    }

    public Optional<DockerImage> getDockerImage() {
        return dockerImage;
    }

    public Optional<String> getVespaVersion() {
        return vespaVersion;
    }

    public Optional<String> getCurrentOsVersion() {
        return currentOsVersion;
    }

    public Optional<String> getHardwareDivergence() {
        return hardwareDivergence;
    }

    public Optional<String> getHardwareFailureDescription() {
        return hardwareFailureDescription;
    }

    public Optional<Boolean> getWantToDeprovision() {
        return wantToDeprovision;
    }

    @Override
    public int hashCode() {
        return Objects.hash(restartGeneration, rebootGeneration, dockerImage, vespaVersion, currentOsVersion,
                hardwareDivergence, hardwareFailureDescription, wantToDeprovision);
    }

    @Override
    public boolean equals(final Object o) {
        if (!(o instanceof NodeAttributes)) {
            return false;
        }
        final NodeAttributes other = (NodeAttributes) o;

        return Objects.equals(restartGeneration, other.restartGeneration)
                && Objects.equals(rebootGeneration, other.rebootGeneration)
                && Objects.equals(dockerImage, other.dockerImage)
                && Objects.equals(vespaVersion, other.vespaVersion)
                && Objects.equals(currentOsVersion, other.currentOsVersion)
                && Objects.equals(hardwareDivergence, other.hardwareDivergence)
                && Objects.equals(hardwareFailureDescription, other.hardwareFailureDescription)
                && Objects.equals(wantToDeprovision, other.wantToDeprovision);
    }

    @Override
    public String toString() {
        return Stream.of(
                        restartGeneration.map(gen -> "restartGeneration=" + gen),
                        rebootGeneration.map(gen -> "rebootGeneration=" + gen),
                        dockerImage.map(img -> "dockerImage=" + img.asString()),
                        vespaVersion.map(ver -> "vespaVersion=" + ver),
                        currentOsVersion.map(ver -> "currentOsVersion=" + ver),
                        hardwareDivergence.map(hwDivg -> "hardwareDivergence=" + hwDivg),
                        hardwareFailureDescription.map(hwDesc -> "hardwareFailureDescription=" + hwDesc),
                        wantToDeprovision.map(depr -> "wantToDeprovision=" + depr))
                .filter(Optional::isPresent)
                .map(Optional::get)
                .collect(Collectors.joining(", ", "NodeAttributes{", "}"));
    }
}
