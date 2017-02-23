#!/bin/bash
astyle --suffix=none --recursive --options=extra/astyle.conf *.cc && \
astyle --suffix=none --recursive --options=extra/astyle.conf *.hpp
