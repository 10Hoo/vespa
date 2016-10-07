package com.yahoo.vespa.model.application.validation;

import com.yahoo.vespa.model.test.utils.VespaModelCreatorWithFilePkg;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import static com.yahoo.vespa.model.application.validation.RankingConstantsValidator.TensorValidationFailed;

public class RankingConstantsValidatorTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Test
    public void ensure_that_valid_ranking_constants_do_not_fail() {
        new VespaModelCreatorWithFilePkg("src/test/cfg/application/validation/ranking_constants_ok/").create();
    }

    @Test
    public void ensure_that_failing_ranking_constants_fails() {
        expectedException.expect(TensorValidationFailed.class);
        expectedException.expectMessage("Tensor coordinate is not a string (VALUE_NUMBER_INT)");
        expectedException.expectMessage("Tensor dimension \"cd\" does not exist");
        expectedException.expectMessage("Tensor dimension \"z\" does not exist");

        new VespaModelCreatorWithFilePkg("src/test/cfg/application/validation/ranking_constants_fail/").create();
    }
}