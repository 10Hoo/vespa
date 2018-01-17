// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.searchdefinition.processing;

import com.yahoo.config.application.api.ApplicationFile;
import com.yahoo.config.application.api.ApplicationPackage;
import com.yahoo.config.model.test.MockApplicationPackage;
import com.yahoo.io.GrowableByteBuffer;
import com.yahoo.io.IOUtils;
import com.yahoo.path.Path;
import com.yahoo.searchdefinition.RankingConstant;
import com.yahoo.searchdefinition.parser.ParseException;
import com.yahoo.tensor.Tensor;
import com.yahoo.tensor.serialization.TypedBinaryFormat;
import com.yahoo.yolean.Exceptions;
import org.junit.After;
import org.junit.Test;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.io.UncheckedIOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * @author bratseth
 */
public class RankingExpressionWithTensorFlowTestCase {

    private final Path applicationDir = Path.fromString("src/test/integration/tensorflow/");
    private final String vespaExpression = "join(rename(reduce(join(Placeholder, rename(constant(Variable), (d0, d1), (d1, d3)), f(a,b)(a * b)), sum, d1), d3, d1), rename(constant(Variable_1), d0, d1), f(a,b)(a + b))";

    @After
    public void removeGeneratedConstantTensorFiles() {
        IOUtils.recursiveDeleteDir(applicationDir.append(ApplicationPackage.MODELS_GENERATED_DIR).toFile());
    }

    @Test
    public void testMinimalTensorFlowReference() throws ParseException {
        StoringApplicationPackage application = new StoringApplicationPackage(applicationDir);
        RankProfileSearchFixture search = new RankProfileSearchFixture(
                application,
                "  rank-profile my_profile {\n" +
                "    first-phase {\n" +
                "      expression: tensorflow('mnist_softmax/saved')" +
                "    }\n" +
                "  }");
        search.assertFirstPhaseExpression(vespaExpression, "my_profile");
        assertConstant("Variable_1", search, Optional.of(10L));
        assertConstant("Variable", search, Optional.of(7840L));
    }

    @Test
    public void testNestedTensorFlowReference() throws ParseException {
        StoringApplicationPackage application = new StoringApplicationPackage(applicationDir);
        RankProfileSearchFixture search = new RankProfileSearchFixture(
                application,
                "  rank-profile my_profile {\n" +
                "    first-phase {\n" +
                "      expression: 5 + sum(tensorflow('mnist_softmax/saved'))" +
                "    }\n" +
                "  }");
        search.assertFirstPhaseExpression("5 + reduce(" + vespaExpression + ", sum)", "my_profile");
        assertConstant("Variable_1", search, Optional.of(10L));
        assertConstant("Variable", search, Optional.of(7840L));
    }

    @Test
    public void testTensorFlowReferenceSpecifyingSignature() throws ParseException {
        StoringApplicationPackage application = new StoringApplicationPackage(applicationDir);
        RankProfileSearchFixture search = new RankProfileSearchFixture(
                application,
                "  rank-profile my_profile {\n" +
                "    first-phase {\n" +
                "      expression: tensorflow('mnist_softmax/saved', 'serving_default')" +
                "    }\n" +
                "  }");
        search.assertFirstPhaseExpression(vespaExpression, "my_profile");
    }

    @Test
    public void testTensorFlowReferenceSpecifyingSignatureAndOutput() throws ParseException {
        StoringApplicationPackage application = new StoringApplicationPackage(applicationDir);
        RankProfileSearchFixture search = new RankProfileSearchFixture(
                application,
                "  rank-profile my_profile {\n" +
                "    first-phase {\n" +
                "      expression: tensorflow('mnist_softmax/saved', 'serving_default', 'y')" +
                "    }\n" +
                "  }");
        search.assertFirstPhaseExpression(vespaExpression, "my_profile");
    }

    @Test
    public void testTensorFlowReferenceSpecifyingNonExistingSignature() throws ParseException {
        try {
            StoringApplicationPackage application = new StoringApplicationPackage(applicationDir);
            RankProfileSearchFixture search = new RankProfileSearchFixture(
                    application,
                    "  rank-profile my_profile {\n" +
                    "    first-phase {\n" +
                    "      expression: tensorflow('mnist_softmax/saved', 'serving_defaultz')" +
                    "    }\n" +
                    "  }");
            search.assertFirstPhaseExpression(vespaExpression, "my_profile");
            fail("Expecting exception");
        }
        catch (IllegalArgumentException expected) {
            assertEquals("Rank profile 'my_profile' is invalid: Could not use tensorflow model from " +
                         "tensorflow('mnist_softmax/saved','serving_defaultz'): " +
                         "Model does not have the specified signature 'serving_defaultz'",
                         Exceptions.toMessageString(expected));
        }
    }

    @Test
    public void testTensorFlowReferenceSpecifyingNonExistingOutput() throws ParseException {
        try {
            StoringApplicationPackage application = new StoringApplicationPackage(applicationDir);
            RankProfileSearchFixture search = new RankProfileSearchFixture(
                    application,
                    "  rank-profile my_profile {\n" +
                    "    first-phase {\n" +
                    "      expression: tensorflow('mnist_softmax/saved', 'serving_default', 'x')" +
                    "    }\n" +
                    "  }");
            search.assertFirstPhaseExpression(vespaExpression, "my_profile");
            fail("Expecting exception");
        }
        catch (IllegalArgumentException expected) {
            assertEquals("Rank profile 'my_profile' is invalid: Could not use tensorflow model from " +
                         "tensorflow('mnist_softmax/saved','serving_default','x'): " +
                         "Model does not have the specified output 'x'",
                         Exceptions.toMessageString(expected));
        }
    }

    @Test
    public void testImportingFromStoredExpressions() throws ParseException, IOException {
        StoringApplicationPackage application = new StoringApplicationPackage(applicationDir);
        RankProfileSearchFixture search = new RankProfileSearchFixture(
                application,
                "  rank-profile my_profile {\n" +
                "    first-phase {\n" +
                "      expression: tensorflow('mnist_softmax/saved', 'serving_default')" +
                "    }\n" +
                "  }");
        search.assertFirstPhaseExpression(vespaExpression, "my_profile");
        assertConstant("Variable_1", search, Optional.of(10L));
        assertConstant("Variable", search, Optional.of(7840L));

        // At this point the expression is stored - copy application to another location which do not have a models dir
        Path storedApplicationDirectory = applicationDir.getParentPath().append("copy");
        try {
            storedApplicationDirectory.toFile().mkdirs();
            IOUtils.copyDirectory(applicationDir.append(ApplicationPackage.MODELS_GENERATED_DIR).toFile(),
                                  storedApplicationDirectory.append(ApplicationPackage.MODELS_GENERATED_DIR).toFile());
            StoringApplicationPackage storedApplication = new StoringApplicationPackage(storedApplicationDirectory);
            RankProfileSearchFixture searchFromStored = new RankProfileSearchFixture(
                    storedApplication,
                    "  rank-profile my_profile {\n" +
                    "    first-phase {\n" +
                    "      expression: tensorflow('mnist_softmax/saved', 'serving_default')" +
                    "    }\n" +
                    "  }");
            searchFromStored.assertFirstPhaseExpression(vespaExpression, "my_profile");
            // Verify that the constants exists, but don't verify the content as we are not
            // simulating file distribution in this test
            assertConstant("Variable_1", searchFromStored, Optional.empty());
            assertConstant("Variable", searchFromStored, Optional.empty());
        }
        finally {
            IOUtils.recursiveDeleteDir(storedApplicationDirectory.toFile());
        }

    }

    /**
     * Verifies that the constant with the given name exists, and - only if an expected size is given -
     * that the content of the constant is available and has the expected size.
     */
    private void assertConstant(String name, RankProfileSearchFixture search, Optional<Long> expectedSize) {
        try {
            Path constantApplicationPackagePath = Path.fromString("models.generated/mnist_softmax/saved/constants").append(name + ".tbf");
            RankingConstant rankingConstant = search.search().getRankingConstants().get(name);
            assertEquals(name, rankingConstant.getName());
            assertEquals(constantApplicationPackagePath.toString(), rankingConstant.getFileName());

            if (expectedSize.isPresent()) {
                Path constantPath = applicationDir.append(constantApplicationPackagePath);
                assertTrue("Constant file '" + constantPath + "' has been written",
                           constantPath.toFile().exists());
                Tensor deserializedConstant = TypedBinaryFormat.decode(Optional.empty(),
                                                                       GrowableByteBuffer.wrap(IOUtils.readFileBytes(constantPath.toFile())));
                assertEquals(expectedSize.get().longValue(), deserializedConstant.size());
            }
        }
        catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private static class StoringApplicationPackage extends MockApplicationPackage {

        private final File root;

        StoringApplicationPackage(Path applicationPackageWritableRoot) {
            this(applicationPackageWritableRoot.toFile());
        }

        StoringApplicationPackage(File applicationPackageWritableRoot) {
            super(null, null, Collections.emptyList(), null,
                  null, null, false);
            this.root = applicationPackageWritableRoot;
        }

        @Override
        public File getFileReference(Path path) {
            return Path.fromString(root.toString()).append(path).toFile();
        }

        @Override
        public ApplicationFile getFile(Path file) {
            return new StoringApplicationPackageFile(file, Path.fromString(root.toString()));
        }

    }

    private static class StoringApplicationPackageFile extends ApplicationFile {

        /** The path to the application package root */
        private final Path root;

        /** The File pointing to the actual file represented by this */
        private final File file;

        StoringApplicationPackageFile(Path filePath, Path applicationPackagePath) {
            super(filePath);
            this.root = applicationPackagePath;
            file = applicationPackagePath.append(filePath).toFile();
        }

        @Override
        public boolean isDirectory() {
            return file.isDirectory();
        }

        @Override
        public boolean exists() {
            return file.exists();
        }

        @Override
        public Reader createReader() throws FileNotFoundException {
            try {
                if ( ! exists()) throw new FileNotFoundException("File '" + file + "' does not exist");
                return IOUtils.createReader(file, "UTF-8");
            }
            catch (IOException e) {
                throw new UncheckedIOException(e);
            }
        }

        @Override
        public InputStream createInputStream() throws FileNotFoundException {
            try {
                if ( ! exists()) throw new FileNotFoundException("File '" + file + "' does not exist");
                return new BufferedInputStream(new FileInputStream(file));
            }
            catch (IOException e) {
                throw new UncheckedIOException(e);
            }
        }

        @Override
        public ApplicationFile createDirectory() {
            file.mkdirs();
            return this;
        }

        @Override
        public ApplicationFile writeFile(Reader input) {
            try {
                IOUtils.writeFile(file, IOUtils.readAll(input), false);
                return this;
            }
            catch (IOException e) {
                throw new UncheckedIOException(e);
            }
        }

        @Override
        public List<ApplicationFile> listFiles(PathFilter filter) {
            if ( ! isDirectory()) return Collections.emptyList();
            return Arrays.stream(file.listFiles()).filter(f -> filter.accept(Path.fromString(f.toString())))
                                                  .map(f -> new StoringApplicationPackageFile(asApplicationRelativePath(f),
                                                                                              root))
                                                  .collect(Collectors.toList());
        }

        @Override
        public ApplicationFile delete() {
            file.delete();
            return this;
        }

        @Override
        public MetaData getMetaData() {
            throw new UnsupportedOperationException();
        }

        @Override
        public int compareTo(ApplicationFile other) {
            return this.getPath().getName().compareTo((other).getPath().getName());
        }

        /** Strips the application package root path prefix from the path of the given file */
        private Path asApplicationRelativePath(File file) {
            Path path = Path.fromString(file.toString());

            Iterator<String> pathIterator = path.iterator();
            // Skip the path elements this shares with the root
            for (Iterator<String> rootIterator = root.iterator(); rootIterator.hasNext(); ) {
                String rootElement = rootIterator.next();
                String pathElement = pathIterator.next();
                if ( ! rootElement.equals(pathElement)) throw new RuntimeException("Assumption broken");
            }
            // Build a path from the remaining
            Path relative = Path.fromString("");
            while (pathIterator.hasNext())
                relative = relative.append(pathIterator.next());
            return relative;
        }

    }

}
