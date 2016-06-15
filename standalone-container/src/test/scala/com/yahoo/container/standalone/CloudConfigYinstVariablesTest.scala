// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.container.standalone

import org.junit.Test
import org.junit.Assert.assertThat
import org.hamcrest.CoreMatchers.is
import org.hamcrest.Matchers.{arrayContaining}

/**
 * @author lulf
 * @author tonytv
 * @since 5.
 */
class CloudConfigYinstVariablesTest {
  def convert = CloudConfigYinstVariables.configServerConverter.convert _

  @Test
  def test_configserver_parsing {
    val parsed = convert("test2-lulf.trondheim.corp.yahoo.com")
    assertThat(parsed.length, is(1))
  }

  @Test
  def port_can_be_configured {
    val parsed = convert("test1-tonyv:123")
    val port: Int = parsed(0).port.get()
    assertThat(port, is(123))
  }

  @Test
  def multiple_spaces_are_supported {
    val parsed = convert("test1     test2")
    assertThat(parsed.size, is(2))

    val hostNames = parsed.map(_.hostName)
    assertThat(hostNames, arrayContaining("test1", "test2"))
  }

  @Test(expected = classOf[IllegalArgumentException])
  def missing_port_gives_exception {
    convert("test1-tonyv:")
  }

  @Test(expected = classOf[IllegalArgumentException])
  def non_numeric_port_gives_exception {
    convert("test1-tonyv:non-numeric")
  }

  @Test
  def string_arrays_are_split_on_spaces {
    val parsed = convert("/home/vespa/foo /home/vespa/bar ")
    assertThat(parsed.size, is(2))
  }

  @Test
  def string_arrays_are_split_on_comma {
    val parsed = convert("/home/vespa/foo,/home/vespa/bar,")
    assertThat(parsed.size, is(2))
  }
}
