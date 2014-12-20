# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Run specific test on specific environment."""

import logging
import os
import sys

from pylib import constants
from pylib.base import base_test_result
from pylib.remote.device import appurify_sanitized
from pylib.remote.device import remote_device_test_run
from pylib.remote.device import remote_device_helper


class RemoteDeviceUirobotRun(remote_device_test_run.RemoteDeviceTestRun):
  """Run uirobot tests on a remote device."""

  DEFAULT_RUNNER_TYPE = 'android_robot'

  def __init__(self, env, test_instance):
    """Constructor.

    Args:
      env: Environment the tests will run in.
      test_instance: The test that will be run.
    """
    super(RemoteDeviceUirobotRun, self).__init__(env, test_instance)

  #override
  def TestPackage(self):
    return self._test_instance.package_name

  #override
  def _TriggerSetUp(self):
    """Set up the triggering of a test run."""
    logging.info('Triggering test run.')
    self._app_id = self._UploadAppToDevice(self._test_instance.apk_under_test)
    if not self._env.runner_type:
      runner_type = self.DEFAULT_RUNNER_TYPE
      logging.info('Using default runner type: %s', self.DEFAULT_RUNNER_TYPE)
    else:
      runner_type = self._env.runner_type
    self._test_id = self._GetTestByName(runner_type)
    config_body = {'duration': self._test_instance.minutes}
    self._SetTestConfig(runner_type, config_body)

  #override
  def _ParseTestResults(self):
    logging.info('Parsing results from remote service.')
    results = base_test_result.TestRunResults()
    if self._results['results']['pass']:
      result_type = base_test_result.ResultType.PASS
    else:
      result_type = base_test_result.ResultType.FAIL
    results.AddResult(base_test_result.BaseTestResult('uirobot', result_type))
    return results
