# Copyright (c) 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script converts to %time% compatible strings passed to it into seconds,
subtracts them, and prints the difference. That's it. It's used by timeit.bat.
"""

import re
import sys

def ParseTime(time_string):
  # Time looks like 15:19:30.32
  match = re.match("(.*):(.*):(.*)\.(.*)", time_string)
  hours, minutes, seconds, fraction = map(int, match.groups())
  return hours * 3600 + minutes * 60 + seconds + fraction * .01

print "%1.2f seconds elapsed time" % (ParseTime(sys.argv[1]) -
                                      ParseTime(sys.argv[2]))
